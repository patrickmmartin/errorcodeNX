Defining and using a symbol analogue for error reporting in c++
===============================================================

Problem statement
--------

High quality software requires a well defined and straightforward error handling strategy to allow a system to protect or check its invariants in the face of unexpected input or runtime state. There are many positions take on how to achieve this see [Google 2015], [Bloomberg 2015] [Mozilla 2015] [Wikipedia 2015]. It can be argued that the issue is not yet a solved problem and the strategies for implementing it in a large system are debated even now.
However, error handing is still everyone's responsibility and particularly so for the applications coded in c++ and C. In this article we will make a proposal, which we'll call ```error_code``` which can be used an _identity_ concept (concept with a little "c") to ensure when a specific course of action is desired, the error state reported by a failed API can be unambiguously recognised at arbitrarily remote call sites. Note that Ruby's symbols [Ruby 2015] and Clojure's keywords [Clojure 2015] supply similar concepts at the language level.

Review of c++ and C approaches
------------------------------

A very common style for reporting from functions in C/c++ is using ```enum``` or ```int``` values or instead encode a value and some additional category info into an integral value like HRESULT. However the ```enum``` and ```int```  have significant issues  when interfaces are composed that have defined their own subsets of return codes, and HRESULT-style values need to be registered, reported and handled consistently.

Consider the case for an ```int``` return status, a library of functions that wishes to consume other library interfaces and report errors without simply passing through values will have to inspect the return codes and itself define yet another set of error values and map from one type to the other. This leads to a proliferation of interfaces and code paths as applications are composed from these libraries. This results in issues from the ongoing maintenance of the code paths mapping errors originating from different libraries for the consumption of various clients.

The situation where an ```enum``` is used is even more problematic as handling these mismatching interfaces requires a switch statement, with the risk of mismapping of new return codes introduced into lib_one_err_t.

### Code Sample: A c++ wrapper that calls another library

```
lib_one_err_t func1;

lib_two_err_t func2()
{
    lib_one_err_t ret1= func1();
    
    if (ret1)
    {
        switch (ret1)
        {
          case LIB_ONE_ERR_TWO:
              return LIB_TWO_ERR_TWO;
          default:
              return LIB_TWO_ERR_UNKNOWN;
        }
    }
    return LIB_TWO_ERR_NONE;
    ....
} 
```

Note that subsequently introduced new error states in ```lib_one_err_t``` will potentially require updates in all places where values of this ```enum``` are examined. The net effect is that interfaces must handle all returned statuses conservatively, which certainly results in less sophisticated program behaviour, which is inefficient, and further opens up the possibility that there is an error in all this additional code.  

If one is to stay with integral types, an HRESULT style approach is a good example of an alternative formulation. This style has the virtue of defining a global namepace of status codes, and allows use of the various bit cracking macros for extracting simple information from identifying individual values. However this approach relies crucially upon the quality of the global registry in a large source base and has typically had mixed results, [citation needed?].

The end result is that without _identity_ the solution tends towards the familiar "a non zero return result indicates an error". Schemes have been constructed to allow additional information relevant to that status to be extracted, but composing them can be difficult or verbose.

Error_code type proposal
------------------------

The proposed type for error_code is this ```typedef const char * error_code```.

Interestingly, a search for prior art in this area reveals no prior suggestions, though we'd love to hear of any we have overlooked TODO(PMM) compiler authors ROFL?. The value is of course unique in the process, being a pointer. Note that it is implementation defined whether identical char[] constants could be folded into one pointer [c++ std lex.string para 13]. In fact, barring inspecting the program core at runtime, this constant folding one of the few ways to obtain the value _a priori_ without having access to the correct symbol in code. Note that so far we have not persuaded any compiler to actually fold two string constants and encounter this issue.

As such, a constant of this type can be used as an _identity_ concept, whose values can be passed through opaquely from any callee to any caller. Caller and callee can be separated by any number of layers of calls and yet the return values can be passed transparently back to a caller, allowing for less mandatory handling, resulting in less effort and less opportunity for erroneous handling. 


### Code sample: defining an error_code

```
error_code foo;

...

// elsewhere

const char foo[] = "My Foo Status";


// you can't do this (no existential forgery)

error_code ret = get_an_error();
if (ret == "My Foo Status") // does not compile with -Wall -Werror "comparison with string literal results in unspecified behaviour" 
{
    ...
}

if (ret == foo) // this is how one must test for equality
{
    ...
}
```

Error_code  desirable properties 
--------------------------------------------------

An _error_code_  has a number of good properties, in addition to being a familiar type to all C/c++ programmers.

* it is a built in type - and has the correct sense: the comparison ```if (error)``` will test for presence of an error condition.
* Safe for concurrent access
* Globally registered, the linker handles it [standard section, dynamic loading etc.] [If patching your binary is your thing, then enjoy, but please aware what implications that process had]
* Yet if the content is a printable string constant, (which we strongly recommend) then inherently it supports printing for logging purposes and can be read for debugging.

Error_code usage examples - "C style"
-------------------------------------

As a consequence of these good properties, we can see the following styles are available.

### Code Sample: manual error handling (Mozilla style)

```
error_code ret;
 
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
error_code ret;
for (test_step : test_steps)
{
    ret = test_step(args);
    if (ret)
    {
        log << "raised error [" << ret << "] in test step " << test_step;
        return error_code;
    }
}
```
 
Use of _error_code_ with exception based error handling
------------------------------------------------------

So far, all the previous code examples would have worked equally well _error_code_ were an integral type, however the identity of an _error_code_ is exploitable to provide good answers to a number of issues that can arise with deciding to use exceptions and also maintains the good properties of exception handling.

Not everyone uses exceptions, and not everyone has used exceptions well; to be fair there has been a history of dubious prior art in this area. All the following are real world examples of code that went to production, or can be found in patent submissions, etc. The names of the guilty parties have been removed while we await the expiry of any relevant statutes.
* ```throw 99```
* ```catch (const char * err)```
* reliance upon `catch (...)```
* reliance upon checking ```what()```

However, making use of error_code and inheriting from one of the standard exception types such that it has the same  identity semantics is useful for the same reasons. There are quite a few techniques one can explore in this direction, but to pick a good example: exception class templates specialised on _error_code_ are very apt:

### Code Sample: exception template allowing exceptions with _identity_

    // we can define a simple template parameterised upon the error_code value
    // this template stores the base type and optionally some additional info if user specified
    // Note that a large chunk of the code is book keeping to keep the instance's data requirement to a minimum
    template <error_code errtype> class typed_error : public std::runtime_exception {
    public:
      typed_error(const char *what = NULL) : _what(NULL) {
        if (what) {
          _what = new std::string(errtype);
          *_what += ". ";
          *_what += what;
        }
      };
      ~typed_error() throw() {
        if (_what)
          delete _what;
      }
      virtual const char *what() const throw() {
        return (_what) ? _what->c_str() : type();
      }
      const char *type() const { return errtype; }
      operator const char *() { return errtype; }
        
    private:
      std::string *_what;
    };    


### Code Sample: define and handle exceptions concisely based upon an _error_code_

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

This approach has some rather neat properties: we can avoid "false matches" caused by code handling caused exception types over eagerly. The parameter has to be an error_code, not a string literal.
Having a unified set of identities allows callees to throw an exception, relying upon callers higher in the stack to make the decision on how to handle that status, and avoiding the need re-throw the exception. Even if re-thrown - if the same _error_code_ is used, the _identity_ is of course preserved even if the stack at the point of re-throw is different from the originating thrower.

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

There is one responsibility that is granted along with the benefit of universality: since everyone could receive an error code, there may be a need to approach declaring _errode_code_ instances to some corporate standard of consistency. This may well mandate some kind of scheme perhaps based upon library, component, etc. to generate standard formats and ensure unique values.

Code Sample: simple example for generating "standard" error_code value.    
```
    #define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str
    
    // this can be used thus
    const char LibA::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");
    
    //which give us the string  "GRP-FOO: Foo not reparable"
    
    // Organisations can exploit other preprocessor features to ensure uniqueness
    #define SCOPE_ERROR_UNIQUE(grp, pkg, error_str) __FILE__ "__LINE__" grp "-" pkg ": " error_str __DATE__ __TIME__ 
```

No Existential Forgery of _error_code_
--------------------------------------

So, what do is conveyed by "existential forgery"?
There are two types
The first is caused innocently enough by interfacing c++ client code a C style API which defins an enum for the return status type from an interface. This *forces* us to make a mapping some status and another - clearly this is a good place for incorrect logic to creep in, on the basis on the nature of the code. 
The second is caused by the problem that integral types are a built in type and have values that can be defined. Hence an undocumented integral API return value from a library can be "handled" by simply performing action for that value, trusting that value. This is by definition not a proven approach. It can also be time consuming to find this kind of problem in a large code base for a specific library as the cognitive load is extremely high. The integral values essentially cannot be made private.

```
case 42:
    // we need to handle this concurrency
    concurrency_action:
```    


For error_code, making the same _faux pas_ this quite simply goes from hard SCOPED_ERROR(...) to (near) impossible SCOPED_ERROR_UNIQUE(...), even with full access to the machinery [ citation needed].


Code Sample: Generation of identities and unique identities
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


What error_code cannot do
-------------------------

No solution is perfect, and this is no exception, so in the spirit of allowing people to choose for themselves, let us list the main concerns one would come up with and address them:

* There is no stable value between processes / builds
    - this is unavoidable and the solution stops working at the boundary of the process - marshalling status codes between different processes, binaries and even different aged builds of the same code cannot rely upon the address. This is a job for some marshalling scheme layered on top of an existing status code system.

* No `switch` statement
    - we do not see this as practically much of an issue as this only applies to code using raw _error_code_, and not exceptions and there are two main use cases:
    1. hand crafting the mapping between incompatible return statuses. For this we suggest it should not be necessary as error_code values would ideally only need to be thrown/returned and consumed 
    2. finally reaching code responsible for handling a set of specific codes differently. In this case, chained ```if/else if``` blocks for integral types should suffice.
    
* Can't return additional info 
  - this is by design as we assume most use cases do not make sense for a pure _error_code_ identity,
  - let us also pause, as this is where the python guys start laughing
      ```(info, error) = attempt_the_thing()``` there will be a suitable analoguous approach in c++

* The exception to the above statement which is a problem shared with the integral values is the classic ```FILE_NOT_FOUND```,  ```TABLE OR VIEW MISSING (but which?)``` return statues, which are a non-trivial source of frustration. If it is useful to attach that parameter, then it seems reasonable that the consumer will need to request a ```pair<error_code, more_info>```, use out parameters or devise some other scheme to compose the required data.

* The strings cannot be translated dynamically
  - In principle displaying the _error_code_ where by design they can be passed around as an opaque value it is a security error to imagine the value should be renderered directly to an end user. Instead in general the system should determine should be shown to whatever the end user, and perform that translation at that point.

Wrap up
-------

In summary, once the perhaps slightly odd feeling of using  _error_code_ fades, we hope it is a technique that people will consider in composing larger systems and when they design the error handling strategy up front.
Because you do do that, right? ;-)


### References

[Google 2015, http://google.github.io/styleguide/cppguide.html ]

[Bloomberg 2015, https://github.com/bloomberg/bde/wiki/CodingStandards.pdf]

[Mozilla 2015, https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style#Error_handling]

[wikipedia 2015, https://en.wikipedia.org/wiki/HRESULT]

[Ruby 2015, http://ruby-doc.org/core-1.9.3/Symbol.html]

[Clojure 2015, http://clojure.org/data_structures#toc8]
