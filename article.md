Defining and using a symbol analogue for error reporting in c++
====

Problem statement
====

High quality software requires a well defined and straightforward
error handling strategy to allow a system to protect or verify its
invariants in the face of invalid input or runtime state. There are
many positions taken on how to achieve this see [Google 2015],
[Bloomberg 2015] [Mozilla 2015] [Wikipedia 2015]. It seems clear
that there is not yet a concensus on the issue.

Nevertheless, error handing is everyone's responsibility and
particularly so for applications coded in c++ and C. In this article
we will make a proposal, which we'll call `error_id` which can be
used an _identity_ concept (concept with a little "c") to ensure
when a specific course of action is desired, the error state reported
by an API can be unambiguously recognised at arbitrarily remote
call sites.

Review of c++ and C approaches
====

A very common style for reporting from functions in C/c++ is using
```enum``` or ```int``` values to enumerate all possible reported
statuses and returning these values from library calls. An extension
of this approach is to encode a value and some additional category
info into an integral value, forming a system of return statuses
like HRESULT [Wikipedia 2015]. However these different set of
independent ```enum``` and ```int``` return values cause significant
issues from the mapping of these independent sets when interfaces
must be composed into new interfaces that themselves must define
new return statuses. HRESULT-style status values do not have this
issue, but a given system must have all possible error return
statuses registered, so that they can be reported and handled
consistently. This scales poorly to larger software system. Note
that in COM/DCOM HRESULTS can be the outcome of IPC calls, thus
extending into other universes of HRESULT values.

It is possible to define even more complex error handling schemes,
such as registering callbacks or having an explicit error stack.
And finally, global or thread-local system library variables for
an `errno` style approach are of course available, with the usual
basket of caveats.

Fundamentally, the problem when library boundaries are crossed is
that without access to a shared _identity_ type whose value describes
the status the solutions all tends towards the familiar "a non zero
return result indicates an error", which is indeed sensibly enough
the position of many error handling schemes employing return codes
exclusively. Schemes have been constructed to allow additional
information relevant to that status to be extracted, but composing
them can be difficult or verbose.

The concern is that a large amount of code becomes devoted to merely
mapping values between the return code sets of various libraries,
which has a number of critiques on how this will scale:

* claiming to handle the return codes from dependency systems (and
  transitively via their dependencies) is a forward commitment, which
  may or not remain valid over time
* the amount of code written / code paths will result in issues
* the most prudent approach is to have a consistent "non-zero return
  code indicates an error" policy, which has the deficiency of requiring
  all library clients "opt into" the steps required to obtain more
  information on a failed operation

error_id type proposal
====

The proposed type for _error_id_ is fundamentally this: `typedef
const char * const error_id` which is the _rvalue_ whereas the
_lvalue_ is of course `typedef const char * error_value`.

We strongly recommend this value should be printable and make sense
in the context of inspecting system state from logs, messages, cores
etc.

Interestingly, a brief search for prior art in this area reveals
no prior proposals, though we'd love to hear of any we have overlooked
(TODO(PMM) compiler authors ROFL?). The value is of course unique
in the process, being a pointer, solving the problem of registration.
Note that it is implementation defined whether identical char[]
constants could be folded into one pointer [c++ std lex.string para
13]. In fact, barring inspecting the program core at runtime, or
having higher order knowledge of the declaration of symbols, this
constant folding one of the few ways to obtain the value _a priori_
without having access to the correct symbol in code. Note that so
far we have not persuaded any compiler to actually fold two string
constants and encounter this issue.

As such, we contend that a constant of this type can be used as an
_identity_ concept, whose values can be passed through opaquely
from any callee to any caller. Caller and callee can be separated
by any number of layers of calls and yet the return values can be
passed transparently back to a caller, allowing for the amount of
mandatory code to be slashed, resulting in less effort and less
opportunity for inappropriate handling.

To compare with prior art in this area: note that Ruby's symbols
[Ruby 2015] and Clojure's keywords [Clojure 2015] supply similar
concepts supported at the language level.

### Code sample: defining an _error_id_

```
// elsewhere, declared
error_id foo;
...
// elsewhere, defined
const char foo[] = "My Foo Status";

...

// you can't do this (no existential forgery)

error_id ret = get_an_error();
/*
if (ret == "My Foo Status") // does not compile with -Wall -Werror
                            //"comparison with string literal results in unspecified behaviour" 
{
    ...
}
*/

if (ret)
{
   if (ret == MY_KNOWN_ERROR) // this is how to test
   {
     // for this interesting case, here we might need to do additional work
     // for logging, notification and the life
   }
   mylogger <<  "api_call returned " ret << "/n";
    
}
return ret;  /// we can always do this with no loss of information
```

_error_id_  desirable properties 
====

An _error_id_  has a number of good properties, in addition to being
a familiar type to all C/c++ programmers.

* it is a built in type _in C and c++_ - and has the expected
  semantics: the comparison ```if (error)``` will test for presence
  of an error condition.
* default use is efficient
* Safe for concurrent access
* usage is exception free 
* Globally registered, the linker handles it [standard section,
  dynamic loading etc.] [If patching your binary is your thing,
  then enjoy, but please aware what implications that process has]
* If the content is a printable string constant, (which we strongly
  recommend) then inherently it supports printing for logging purposes
  and can be read for debugging.

error_id usage examples - "C style"
====

As a consequence of these good properties, we can see the following styles are available.

### Code Sample: manual error handling (Mozilla style)

```
error_value ret;
 
ret = in();
if (ret)
    return ret;
    
ret = a_galaxy();
if (ret)
    return ret;
    
ret = far_far_away();
if (ret)
    return ret;

ret = awaken_force();
    
if (ret)
{
     // list known continuable errors here
     if ((ret == E_FORD_BROKEN_ANKLE) || (ret == E_FORD_TRANSPORTATION_INCIDENT) &&
         (Ford::scenes::in_the_can()))
     {
        print_local_failure(ret); // whoops!
     }
     else
     {
         panic(ret); // THIS FUNCTION DOES NOT RETURN
     }
}

order_popcorn();
```

### Code Sample: error conditions can be composed dynamically

```
error_value ret;
for (test_step : test_steps)
{
    ret = test_step(args);
    if (ret)
    {
        log << "raised error [" << ret << "] in test step " << test_step;
        return error_id;
    }
    // alternatively we might run all,
    // or more and produce a nicely formatted table for debugging / monitoring
    
}
```
 
Use of _error_id_ with exception based error handling
------------------------------------------------------

So far, all the previous code examples would have worked almost as
well if _error_id_ were an integral type, however the identity of
an _error_id_ is exploitable to provide good answers to a number
of issues that can arise with deciding to use exceptions and also
maintains the good properties of exception handling.

Not everyone uses exceptions, and not everyone has used exceptions
well; to be fair there has been a history of dubious prior art in
this area. All the following are real world examples of code that
went to production, or can be found in patent submissions, etc. The
names of the guilty parties have been removed while we await the
expiry of any relevant statutes.
* ```throw 99```
* ```catch (const char * err)```
* reliance upon `catch (...)```
* reliance upon checking ```what()```
* every exception is std::runtime_error

However, making use of _error_id_ and inheriting from one of the
standard exception types such that it has the same identity semantics
is useful for the same reasons as for the using the raw value. As
an example: exception class templates specialised on _error_id_ are
very apt:

### Code Sample: exception template allowing exceptions with _identity_
  
```  
// we can define a simple template parameterised upon the error_id value
template <error_id errtype> class typed_error_lite : public std::exception {};
    
// or we can go a little further and allow for some additional information
// this one has a base type and additional info
template <error_id errtype> class typed_error : public std::exception {
public:
  typed_error(const char *what = NULL) : _what(what) {}

  virtual const char *what() const throw() { return (_what) ? _what : type(); }
  const char *type() const { return errtype; }
  operator const char *() { return errtype; }

private:
  error_id _what;
};
```

### Code Sample: define and handle exceptions concisely based upon an _error_id_

```
    // elsewhere
    FooErrors::EFOO = "FOOlib: Foo error";
    
    ...

    // we can define new unique exception instances
    typedef typed_error<FooErrors::EFOO> foo_err;
    typedef typed_error<FooErrors::EBAR> bar_err;
    
    try
    {
      // something that throws a typed_error
    }
    catch (typed_error<FooErrors::EFOO> &e)
    {
      // use the template
    }
    /* you can't even write this 
    catch (typed_error<"FOOlib: Foo error"> &e)
    {
      // use the template
    }
    */
    catch (bar_err &e)
    {
      // or a typedef
    }
    catch (...)
    {
      // we don't get here
    }
```

This approach has some rather neat properties: we can avoid "false
matches" caused by code handling exception types too greedily. The
parameter has to be an _error_id_, not a string literal.

Having a unified set of identities allows callees to throw an
exception, relying upon callers higher in the stack to make the
decision on how to handle that status, and avoiding the need re-throw
the exception. Even if re-thrown - if the same _error_id_ is used,
the _identity_ is of course preserved even if the stack at the point
of re-throw is different from the originating thrower.

### Code Sample: exception handling with fall-through

```
    try
    {
      // something that throws a typed_error<LibA::EPOR>
      // if LibA::EPOR ia not a publically visible value,
      // it is not possible to write a handler for that case only
      // nor throw one, except for the code owning that identity
    }
    catch (typed_error<LibA::EBAR> &e)
    {
      // not caught
    }
    catch (std::runtime_error &e)
    {
      // typed_error<LibA::EPOR> is caught here, conveniently
    } 
```

TODO(PMM) - think about possible template instantiation concerns.

There is one responsibility that is granted along with the benefit
of universality: since everyone could receive an error code, there
may be a need to approach declaring _error_id_ instances to some
corporate standard of consistency. This may well mandate some kind
of scheme perhaps based upon library, component, etc. to generate
standard formats and ensure unique values.

### Code Sample: simple example for generating "standard" _error_id_ value.    
```
#define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str

// this can be used thus
const char LibA::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

// which give us the string  "GRP-FOO: Foo not reparable"

// Organisations can exploit other preprocessor features to ensure uniqueness
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SCOPE_ERROR_UNIQUE(grp, pkg, error_str) \
    __FILE__ ":" TOSTRING(__LINE__) " " grp "-" pkg ": " error_str " " __DATE__ " " __TIME__

// which give us a string like ../test_error_id.cpp:39 GRP-FOO: Foo not Bar Oct  4 2015 12:07:30

```

No Existential Forgery of _error_id_
--------------------------------------

So, what do is meant by "existential forgery"? There are two types:
* the first is caused innocently enough by interfacing c++ client
  code a C style API which defins an enum for the return status type
  from an interface. This *forces* us to make a mapping some status
  and another - clearly this is a good place for incorrect logic to
  creep in, on the basis on the nature of the code.
* the second is caused by the problems caused by a policy of not
  documenting return values; integral values cannot be made an
  implementation detail and in large systems it is all too common for
  code to handle the return `-99` to appear when clients perceive a
  need to perform handling for that situation.

This problem is addressed by _error_id_ in two different ways:
 - possibly most valuably, we can break out of the cycle because
   the moderate level of self-description in the string of the raw
   value should faciliate implementing a better approach as trivially
  the component, file and issue can be delivered
 - additionally, _error_id_ values can be made internal or for
   public consumption, enforcing a consistent discipline using the
   language, again the string contents can back this up, but the clear
   contract supplied by the library can be "please feel free to
   interpret these error states, but any others must be handled using
   the "Internal Error" strategy
 - note also that exposing an _error_id_ is no longer a forward
   commitment, as prior values can be *removed* in new revisions of
   the interface, in addition to new ones being introduced. This is
   of course in constrast to integral values return codes.


### Code Sample: Generation of identities and unique identities
```
const char N::new_bar[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
 
assert(strcmp(N::new_bar, FooErrors::EBAR) == 0);
assert((N::new_bar != FooErrors::EBAR));
 
try
{
  throw typed_error<N::new_bar>("bazong not convertible to bar");
}
catch (typed_error<FooErrors::EBAR> &e)
{
  assert(false, "in typed_error<FooErrors::EBAR> handler");
}
catch (typed_error<N::new_bar> &e)
{
  // ok!
}
catch (...)
{
  assert(false, "Fell through to catch all handler");
}
```

What _error_id_ cannot do
-------------------------

No solution is perfect, and this is no exception, so in the spirit
of allowing people to choose for themselves, let us attempt to list
some of common concerns one would come up with and address them:

* be cases in ```switch``` statements
    - we do not see this as practically much of an issue as this
       only applies to code using raw _error_id_, and not exceptions
       and there are two main use cases:
    1. hand crafting the mapping between incompatible return statuses.
       For this we suggest it should not be necessary as _error_id_
       values would ideally only need to be thrown/returned and consumed
    2. finally reaching code responsible for handling a set of
       specific codes differently. In this case, chained ```if/else
       if``` blocks for integral types should suffice.
    
* return additional info 
  - this is of course a natural consequence of using a pure _error_id_
     as an identity
  - exception classes similar to _typed_error_ of course allow as
     much context as one is prepared to pay for in one object instance,
  - if status need more context - conditions like "[file|table|foo]
     not found" being the most infuriating - then we have to leave it
     to the user to code up a solution to pass back more context

* defend against abuse
  - in a C / c++ application, there is no way to completely prevent
    abuse such as _error_id_ values being appropriated and used
    inappropriately; the intent of the proposal is to illustrate the
    benefits arising from the simplicity and effectiveness of using
    _error_id_. We hope that the solution would be adopted widely
    upon its own merits.

* yield stable values between processes / builds
    - firstly, it should be remembered the value is not be inspected - only what it points to 
    - secondly, this is unavoidable and the solution of course stops
       working at the boundary of the process - marshalling status
       codes between different processes, binaries and even different
       aged builds of the same code cannot rely upon the address. This
       is a job for some marshalling scheme layered on top of an
       existing status code system.

* be used as translatable strings
  - the most important point to make here is these strings should
     never be treated as trusted output safe to be displayed to system
     end users. An _error_id_ can travel as an opaque value, and hence
     there is no rigorous mechanism that could preventing information
     leakage
  - finally _error_id_ is a pointer to a const array of char: dynamic
     translation into user readable strings can only be done by mapping
     values to known tranlsations. For even a modest size system it
     becomes more effective to have a facility for text translation
     which would offer more features relevant  to that task that just
     an _error_id_.

Wrap up
=======

In summary, once the perhaps slightly odd feeling of using  _error_id_
fades, we hope it is a technique that people will adopt in composing
larger systems when the error handling strategy is being designed.
The process wide identity concept allows for composition of large-scale
applications comprising many components, while affording the
ooportunity of an exception-like error handling with or _without_
employing actual exceptions, and maintaining a high level of
debuggability. This approach will allow both C and c++ libraries
to become first class citizens in a design where error handling
need never be left to chance or assumption.

Recommendations
====

* Define your identities for system states
* Define how you wish to expose these identities and distribute
   them. For example, component level, subsystem level, applciation
   wide?
* Use them rigorously

Curate's Eggs
-------------

There are yet some potentially interesting ramifications that fall
out from error_id that have not been demonstrated in the interest
of brevity, but which we'll touch upon here to pique your interest.

* _missing switch_: it is possible to write template metaprograms that will
    - allow statically typed handlers to be registered for a switch
       statement to ensure values are always handled, with various
       outcomes for a fall-through, (Fail, Pass, etc.)
    - even prevent compilation if handlers are not located for
       specific  _error_id_ instances
* _private values_: it is possible to define an _error_id_
   _typed_error_  with a value not visible to clients
   - for _typed error_ this would allows a standard "abort via
      exception" for reporting those error conditions not understood
      explicitly by callers
   - for _error id_ this can allow a hierarchy of error conditions
      to be defined
* _reserved values_: it is possible to expose an _error_id_ such
   that it can not be used to define a _typed_error_, yet the value
   can still be used


### References

[Google 2015, http://google.github.io/styleguide/cppguide.html ]

[Bloomberg 2015, https://github.com/bloomberg/bde/wiki/CodingStandards.pdf]

[Mozilla 2015, https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style#Error_handling]

[wikipedia 2015, https://en.wikipedia.org/wiki/HRESULT]

[Ruby 2015, http://ruby-doc.org/core-1.9.3/Symbol.html]

[Clojure 2015, http://clojure.org/data_structures#toc8]
