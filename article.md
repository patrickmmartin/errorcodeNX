Identify your Errors better with char[]
====

Abstract
====

The use of exceptions isn't a viable error handling approach in all
cases and returning codes for error handling is sometimes preferable.
However, using integral types for error identification is problematic
as there is no good mechanism to guarantee that each value uniquely
identifies a specific error. This article proposes the use of
`char` arrays instead as these are unique straight away. In addition
`char` arrays also yield a simple way to get an indication of the
cause of an error.

Problem statement
====

High quality software requires a well defined and straightforward
error handling strategy to allow a system to protect or verify its
invariants in the face of invalid input or runtime state. There are
many positions taken on how to achieve this (see [Google 2015],
[Bloomberg 2015] [Mozilla 2015] [Wikipedia 2015]). It seems clear
that there is not yet a consensus on the issue.

Nevertheless, error handing is everyone's responsibility and
particularly so for applications coded in C++ and C. In this article
we will make a proposal, which we'll call `error_id`, that can be
used as an _identity_ concept (concept with a little "c") to ensure
when a specific course of action is desired, the error state reported
by an API can be unambiguously recognised at arbitrarily remote
call sites.

Review of C++ and C approaches
====

A very common style for reporting from functions in C and C++ is using
```enum``` or ```int``` values to enumerate all possible reported
statuses and returning these values from library calls. An extension
of this approach is to encode a value and some additional category
info into an integral value, forming a system of return statuses
like ```HRESULT``` [Wikipedia 2015]. However these different sets
of independent ```enum``` and ```int``` return values cause significant
issues from the mapping of these independent sets when interfaces
must be composed into new interfaces that themselves must define
new return statuses. ```HRESULT```-style status values do not have
this issue, but a given system must have all possible error return
statuses registered, so that they can be reported and handled
consistently. This scales poorly to larger software system. Note
that in COM/DCOM ```HRESULT```s can be the outcome of IPC calls,
thus extending into other universes of ```HRESULT``` values.

It is possible to define even more complex error handling schemes,
such as registering callbacks or having an explicit error stack.
And finally, global or thread-local system library variables for
an `errno` style approach are of course available, with the usual
basket of caveats.

Fundamentally, the problem when library boundaries are crossed is
that without access to a shared _identity_ type whose value describes
the status the solutions all tends towards the familiar "a non zero
return indicates an error", which is sensibly enough indeed
the position of many error handling schemes employing return codes
exclusively. Schemes have been constructed to allow additional
information relevant to that status to be extracted, but composing
them can be difficult or verbose.

The concern is that a large amount of code becomes devoted to merely
mapping values between the return code sets of various libraries;
this has a number of critiques on how this will scale:

* claiming to handle the return codes from dependency systems (and
  transitively via their dependencies) is a forward commitment, which
  may or may not remain valid over time
* the amount of code written / code paths will result in issues
* the most prudent approach is to have a consistent "non-zero return
  code indicates an error" policy, which has the deficiency of requiring
  all library clients "opt into" the steps required to obtain more
  information on a failed operation

C++11's `std::error_code`
====

C++11 introduced an interesting approach to deal with errors through
the classes `std::error_code` and `std::error_condition`: the idea
of these classes is that error values are represented as enumerators
with the enum type identifying an error category.  The difference
between`std::error_code` and `std::error_condition` is that the
former is for implementation specific handling of errors while the
latter is portable handing of error messages. Although the two
classes are different we only refer to `std::error_code` below: the
same concepts apply to `std::error_condition`.

Each enum type used with an `std::error_code` is mapped to a different
error category.  An error category is a class derived from `std::error_category`.
An `std::error_code` object holds an enumerator value stored as
`int` and a reference to the corresponding `std::error_category`.
When comparing `std::error_code` objects both the stored `int` and
the `std::error_category` can be taken into account, allowing for
a mechanism to create unique `std::error_code` values.

The standard C++ library defines error enums and corresponding
`std::error_category`s defined for typical system domain.  Users
can create new error enums and corresponding `std::error_category`s
to cover non-standard errors. Unfortunately, creating new error
categories is relatively involved: an enum needs to be created and
registered as an error enum by specializing `std::is_error_code_enum<E>`,
an error category needs to be created by inheriting from
`std::error_category` and implementing its pure virtual functions,
and `std::make_error_code()` needs to be overloaded for the error
enum. 

Although `std::error_code` can address uniqueness of errors propagated
in a system, its use is unfortunately fairly complicated. Especially
when people are not easily convinced that meaningful errors need
to be returned a much simpler approach is needed.

error_id type proposal
====

The proposed type for `error_id` is`typedef char const error_id[]`
which is the _error constant_ whereas the
_variable_ is of course `typedef char const* error_value`. The idea
is that each `error_id` is made unique by the linker in a process
without any need of registration. Note that functions returning an
`error_id` need to be declared to return an `error_value` because
functions can't return arrays.

We strongly recommend this value should be printable and make sense
in the context of inspecting system state from logs, messages, cores
etc.

Interestingly, a brief search for prior art in this area reveals
no prior proposals, though we'd love to hear of any we have overlooked.
An honourable mention should be made of the second example in the
_rogues' gallery_ of exception anti-patterns we present later 
as that author was part of the way towards our proposal.

As such, we contend that a constant of this type can be used as an
_identity_ concept, whose values can be passed through opaquely
from any callee to any caller. Caller and callee can be separated
by any number of layers of calls and yet the return values can be
passed transparently back to a caller without any need of translating
error codes, resulting in less effort and less opportunity for
inappropriate handling.

To compare with prior art in this area: note that Ruby's symbols
[Ruby 2015] and Clojure's keywords [Clojure 2015] supply similar
concepts supported at the language level.

### Code sample: defining an `error_id`

```
// declaration of an error_id:
extern error_id MY_KNOWN_ERROR;
extern error_value get_an_error();
...
// definition of an error_id:
error_id MY_KNOWN_ERROR = "My Foo Status";

...

// you can't do this (no existential forgery)

error_value ret = get_an_error();
/*
if (ret == "My Foo Status") // does not compile
                            // with -Wall -Werror
                            // "comparison with 
                            // string literal
                            // results in 
                            // unspecified
                            // behaviour" 
{
    ...
}
*/

if (ret)
{
   if (ret == MY_KNOWN_ERROR) // this is how to test
   {
     // for this interesting case, here we might
     // need to do additional work
     // for logging, notification and the like
   }
   mylogger <<  "api_call returned " << ret << "\n";
    
}
return ret;  // we can always do this with no
             // loss of information
```

`error_id` desirable properties 
====

An `error_id` has a number of good properties, in addition to being
a familiar type to all C and C++ programmers.

* it is a built in type _in C and C++_ - and has the expected
  semantics: the comparison ```if (error)``` will test for presence
  of an error condition.
* default use is efficient
* safe for concurrent access
* usage is exception free 
* globally registered, the linker handles it [If patching your binary
  is your thing, then enjoy, but please beware what implications that
  process has]
* if the content is a printable string constant, (which we strongly
  recommend) then it inherently supports printing for logging purposes
  and can be read for debugging. Sadly, printing the "missing `error_id`"
  (`NULL`) results in undefined behaviour for streams and
  `printf`, i.e., upon success the "error result" can't be printed
  directly.

For the last point it is helpful to use a simply utility function which
arranges to turn the result into always printable values:

```
error_value error_string(error_value ret) {
    return ret? ret: "<no error>";
}
```

`error_id` usage examples - "C style"
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
     if ((ret == e_FORD_BROKEN_ANKLE) ||
         (ret == e_FORD_TRANSPORTATION_INCIDENT) &&
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
        log << "raised error [" << ret << "] " 
               "in test step " <<
               test_step << '\n';
        return ret;
    }
    // alternatively we might run all,
    // or more and produce a nicely formatted
    // table for debugging / monitoring
    
}
```
 
Use of `error_id` with exception based error handling
------------------------------------------------------

So far, all the previous code examples would have worked almost as
well if `error_id` were an integral type, however the identity of
an `error_id` is exploitable to provide good answers to a number
of the issues that come with using exceptions while retaining the known
established good practices and features of exception handling.

Not everyone uses exceptions, and not everyone has used exceptions
well; to be fair there has been a history of dubious prior art in
this area. All the following are real world examples of code that
went to production, or can be found in patent submissions, etc. The
names of the guilty parties have been removed while we await the
expiry of any relevant statutes.
* ```throw 99```
* ```catch (const char * err)```
* reliance upon ```catch (...)```
* reliance upon checking ```what()```
* every exception is ```std::runtime_error```

However, making use of `error_id` while simultaneously inheriting from
a standard exception class in the `std::exception` hierarchy 
is useful for the same reasons as for using the raw value.
As an example: exception class templates specialised on `error_id` are
very apt:

### Code Sample: exception template allowing exceptions with _identity_
  
```  
// we can define a simple template parameterised upon the error_id value
template <error_id errtype>
class typed_error_lite : public std::exception {};
    
// or we can go a little further and allow for some additional information
// this one has a base type and additional info
template <error_id errtype>
class typed_error : public std::runtime_error {
public:
  typed_error(const char* what = errtype): 
  			  std::runtime_error(what) {}

  const char *type() const { return errtype; }
  operator const char *() { return errtype; }
};
```

### Code Sample: define and handle exceptions concisely based upon an `error_id`

```
// somewhere
struct FooErrors {
    static constexpr error_id eFOO =
					 "FOOlib: Foo error";
    static           error_id eBAR;
    //...
};
// elsewhere
constexpr error_id FooErrors::eFOO;
	      // a definition is still required
error_id FooErrors::eBAR = "FOOlib: Bar error";

...

// we can define new unique exception instances
typedef typed_error<FooErrors::eFOO> foo_err;
typedef typed_error<FooErrors::eBAR> bar_err;

void f() {    
    try
    {
      // something that throws a typed_error
    }
    catch (typed_error<FooErrors::eFOO> const &e)
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
}
```

This approach has some rather neat properties: we can avoid "false
matches" caused by code handling exception types too greedily. The
parameter has to be an`error_id`, not a string literal.

Having a unified set of identities allows callees to throw an
exception, relying upon callers higher in the stack to make the
decision on how to handle that status, and avoiding the need to re-throw
the exception. Even if re-thrown - if the same `error_id` is used,
the _identity_ is of course preserved even if the stack at the point
of re-throw is different from the originating thrower.

### Code Sample: exception handling with fall-through

```
    try
    {
      // something that throws a
      // typed_error<LibA::ePOR>
      // if LibA::ePOR is not a
      // publically visible value,
      // it is not possible to write
      // a handler for that specific case 
      // nor throw one, except for the
      // code owning that identity
    }
    catch (typed_error<LibA::eBAR> &e)
    {
      // not caught
    }
    catch (std::runtime_error &e)
    {
      // typed_error<LibA::ePOR> is caught here, conveniently
    } 
```

There is one responsibility that is granted along with the benefit
of universality: since everyone could receive an error code, there
is a need to approach declaring `error_id` instances to some standard
of consistency. This may well require defining a scheme to generate
standard formats and help ensure unique values, perhaps based upon
library, component, etc. - thus:

### Code Sample: simple example for generating "standard" `error_id` value.    
```
#define SCOPE_ERROR(grp, pkg, error_str) \ 
                    grp "-" pkg ": " error_str

// this can be used thus
const char LibA::ePOR[] =
  SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

// which give us the string:
// "GRP-FOO: Foo not reparable"

// Organisations can exploit other preprocessor
// features to ensure uniqueness of output
#define TOSTRING(x) #x

#define SCOPE_ERROR_LOCATION(grp, pkg, error_str)                                \
		__FILE__ ":" TOSTRING(__LINE__) " " grp "-" pkg ": " error_str " "

// which give us a string like 
// ../test_error_id.cpp:39 GRP-FOO: Foo not Bar
```

In summary, the primary risk from identical strings in
two logically distinct `error_id` declarations is when these `error_id`
symbols need to be distinguishable by some calling code when an `error_value` may
receive a value of either identity. `error_id` does not have an issue and
"does the right thing" from the viewpoint of reading the code. However it should
be remembered the identity of an `error_id` is intended to derive entirely from
its _content_, and in the prior case, the printed values will be the same,
further reinforcing the utility of a rule requiring `error_id` content which is
printable and distinct for each unique identity.

No Existential Forgery of `error_id`
--------------------------------------

So, what is meant by "existential forgery"? There are two types:
* the first is caused innocently enough by interfacing C++ client
  code with a C style API which define an enum for the return status type
  from an interface. This *forces* us to make a mapping some status
  to another - clearly this is a good place for incorrect logic to
  creep in, on the basis on the nature of the code.
* the second is caused by the problems caused by a policy of not
  documenting return values; integral values cannot be made an
  implementation detail and in large systems it is all too common for
  code to handle the return `-99` to appear when clients perceive a
  need to perform handling for that situation.

This problem is addressed by `error_id` in multiple ways:
 - possibly most valuably, we can break out of the cycle because
   the moderate level of self-description in the string of the raw
   value should facilitate implementing a better approach as trivially
   the component, file and issue can be delivered
 - additionally, `error_id` values can be made internal or for
   public consumption, enforcing a consistent discipline using the
   language, again the string contents can back this up, but the clear
   contract supplied by the library can be "please feel free to
   interpret these error states, but any others must be handled using
   the "Internal Error" strategy
 - note also that exposing an `error_id` is no longer a forward
   commitment to support that value for all future time, 
   in contrast to integral value return codes, as prior values can
   be *removed* in new revisions of the interface, in addition to
   new ones being introduced and clients addressing removed values
   will simply fail to compile.


### Code Sample: Generation of identities and unique identities
```
const char N::new_bar[] =
    SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
 
assert(strcmp(N::new_bar, FooErrors::eBAR) == 0);
assert((N::new_bar != FooErrors::eBAR));
 
try
{
  throw typed_error<N::new_bar>("bazong not convertible to bar");
}
catch (typed_error<FooErrors::eBAR> &e)
{
  assert(0 == "in typed_error<FooErrors::eBAR> handler");
}
catch (typed_error<N::new_bar> &e)
{
  // ok!
}
catch (...)
{
  assert(0 == "Fell through to catch all handler");
}
```

What `error_id` cannot do
-------------------------

No solution is perfect, and this approach is no exception. In the spirit
of allowing people to choose for themselves, let us attempt to list
some of the common concerns one would come up with and address them:

* `error_id`s cannot be cases in ```switch``` statements
    - we do not see this as much of an issue as this restriction
       only applies to code using raw`error_id`, and not exceptions
       and there are two main use cases:
    1. hand crafting the mapping between incompatible return statuses.
       This use should not be necessary as `error_id`
       values would ideally only need to be thrown/returned and consumed
    2. finally reaching code responsible for handling a set of
       specific codes differently. In this case, chained ```if/else
       if``` blocks for integral types should suffice.
    
* return additional info 
  - this restriction is of course a natural consequence of using a pure
    `error_id` as an identity
  - exception classes similar to `typed_error` of course allow as
     much context as one is prepared to pay for in each object instance
  - if status need more context - conditions like "[file|table|foo]
     not found" being the most infuriating - then we have to leave it
     to the user to code up a solution to pass back more context. The
     same restriction applies when using integral results.

* defend against abuse
  - in a C / C++ application, there is no way to completely prevent
    abuse such as `error_id` values being appropriated and used
    inappropriately; the intent of the proposal is to illustrate the
    benefits arising from the simplicity and effectiveness of using
   `error_id`. We hope that the solution would be adopted widely
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
  - the most important point to make here is that these strings should
     never be treated as trusted output safe to be displayed to system
     end users. An `error_id` can travel as an opaque value, and hence
     there is no rigorous mechanism that could prevent information
     leakage
  - finally `error_id` is a pointer to a const array of char: dynamic
     translation into user readable strings can only be done by mapping
     values to known translations. For even a modest size system it
     becomes more effective to have a facility for text translation
     which would offer more features relevant  to that task than just
     an`error_id`.

Comparison of `error_id` and `std::error_code`
====

The proposed `error_id` and `std::error_code` have some common
features. In particular, both address error propagation with and
without exceptions, both provide uniquely identified errors, and
both can be globally consumed without requiring declarations of
all errors.

There are also important differences. For `std::error_code` the
category yields one level or hierarchical grouping while there is
no grouping of `error_id`s at all. On the other hand, creating new
errors with `std::error_code`s requires definition of multiple
entities while creating an `error_id` is just one definition. If
the respective specific error should be detectable by users suitable
declarations in headers are needed in both cases.

A major difference is that `error_id` can be used both with C and
C++ code while `std::error_code` only works with C++ code.  In code
bases where different languages are used in the same executable it
is helpful to use an error reporting scheme available to all of
these languages.

Wrap up
=======

In summary, once the perhaps slightly odd feeling of using  `error_id`
fades, we hope it is a technique that people will adopt in composing
larger systems when the error handling strategy is being designed.
The process wide identity concept allows for composition of large-scale
applications comprising many components, while affording the
opportunity of an exception-like error handling with or _without_
employing actual exceptions, and maintaining a high level of
usability for debugging. This approach will allow both C and C++ libraries
to become first class citizens in a design where error handling
need never be left to chance or assumption.

Please note that it is implementation defined whether identical string literals use
identical or different addresses [c++ std lex.string para 13]. In fact, constant folding
where one string literal becomes the data for many identical declarations of string literals
in code occurs in many current compilers. Hence it is key to use char arrays where this does not happen.

Recommendations
====

* Define your identities for system states
* Define how you wish to expose these identities and distribute
   them. For example, component level, subsystem level, application
   wide?
* Use them rigorously!

Curate's Eggs
-------------

There are yet some potentially interesting ramifications that fall
out from `error_id` that have not been demonstrated in the interest
of brevity, but which we'll touch upon here to pique your interest.

* _missing switch_: it is possible to write template metaprograms that will
    - allow statically typed handlers to be registered for a switch
       statement to ensure values are always handled, with various
       outcomes for a fall-through, (Fail, Pass, etc.)
    - even prevent compilation if handlers are not located for
       specific `error_id` instances
* _private values_: it is possible to define a `error_id` with an
    value not visible to clients
   - for _typed error_ this would allows a standard "abort via
      exception" for reporting those error conditions not understood
      explicitly by callers
   - for raw _error id_ this can allow a crude hierarchy of error conditions
      to be defined

Footnotes
---------

Michael Maguire discovered that due to an apparent compiler bug in IBM's xlC V11.1 
the arrays of unspecified size `char[]` need to be explicitly decayed.
The fix is to use `+eFOO` instead of `eFOO` when `eFOO` is to be passed to a
function template.


### References

Code illustrating the concept can be found at https://github.com/patrickmmartin/errorcodeNX/tree/article_nov_2015

[Google 2015, http://google.github.io/styleguide/cppguide.html ]

[Bloomberg 2015, https://github.com/bloomberg/bde/wiki/CodingStandards.pdf]

[Mozilla 2015, https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style#Error_handling]

[wikipedia 2015, https://en.wikipedia.org/wiki/HRESULT]

[Ruby 2015, http://ruby-doc.org/core-1.9.3/Symbol.html]

[Clojure 2015, http://clojure.org/data_structures#toc8]
