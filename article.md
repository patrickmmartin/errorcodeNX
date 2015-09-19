Defining and using a symbol analogue for error reporting in c++
===============================================================

Problem statement
--------

High quality software requires a well defined and straightforward error handling strategy to allow a system to protect or check its invariants in the face of unexpected input or runtime state. There are many positions take on how to achieve this see [Google 2015], [Bloomberg 2015] [Mozilla 2015] [Wikipedia 2015]. It can be argued that the issue is not yet a solved problem and the strategies for implementing it in a large system are debate  even  at this point in time.
However, error handing is still everyone's responsibility and arguably particularly so for the kinds of applications that tend to be coded in c++ and C. In this article we will make a proposal, which we'll call ```error_code``` which can be used an identity concept (with a little "c") to ensure when a specific course of action is desired, the error state reported by a failed API can be unambiguously recognised at arbitrarily remote call sites. Note that Ruby's symbols [Ruby 2015] and Clojure's keywords [Clojure 2015] supply similar concepts at the language level.

Review of c++ and C approaches
------------------------------

A very common style in C is using ```enum``` or ```int``` TODO citations, HRESULT   . But the former has significant issues, where interfaces are composed that have defined their own subsets of return codes, and HRESULT-style values need to be registered, reported and handled consistently.

In the case that a ```int``` is used, without a strictly enforced and maintained global registry of values, a library of functions that needs to consume yet other libtrary interfaces and report errors encountered in its processing will have to inspect the return codes and itself define yet another set of error values. This leads to a proliferation of interfaces and code paths as applications are composed from these libraries. There is also the issue of ongoing maintenance of the code paths mapping errors originating from  different libraries for the consumption of various clients. And of cource, unless some mechanism is in place to capture the original error, the original identity of the error state is entirely lost.
The situation where an ```enum``` is used is even more problematic as in c++ handling these mismatching interfaces requires a switch statement, with the risk of mismapping of new return codes introduced into lib_one_err_t.
If one is to stay with integral types, an HRESULT style approach is a good example of an alternative formulation. This style has the strength of defining a gobal identity, though beyond using the various bit cracking macros for simple information, for identifying individual values this relies crucially upon the quality of the global registry over a large source base and has typically had mixed results, [citation needed?] 


For example, here is a simple constructed example of a wrapper that calls into another library in c++:

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

Error_code type proposal
------------------------

The proposed type for error_code is this ```typedef const char * error_code```. TODO(PMM) STD: type for string constants!

Interestingly, a search for prior art in this area reveals no prior suggestions, though we'd love to hear of any we have overlooked. The value is unique in the process, and as such can be used as an _identity_ concept, whose values can be passed through opaquely from any callee to any caller. Caller and callee can be separated by any number of layers of calls and yet the return values can be passed transparently back to a caller, allowing for less mandatory handling, resulting in less effort and less opportunity for erroneous handling. 

TODO(PMM) example definition and declarations (and what won't work - literals _existential forgery_ )

Error_code  desirable properties 
--------------------------------------------------

An ```error_code```  has a number of good properties.
* it's built in type - the comparison ```if (error)``` will test for an error
* Globally registered, the linker handles it [standard section, dynamic loading etc.] [If patching your binary is your thing, then enjoy, but please aware what implications that process had]
* Can be used as an opaque value
* Yet if a printable  string constant, (which we strongly recommend) then inherently it supports printing for logging purposes.



Error_code usage examples - "C style"
-------------------------------------

As a result of these good properties, we can see the following styles are available.
Example: error conditions can be composed manually with little effort and a clean style

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

Example: error conditions can now be composed dynamically:

```
error_code ret;
for (test_step : test_steps)
{
    ret = test_step(args)
    if (!ret)
    {
        log << "raised error [" << ret << "] in test step " << test_step;
        return error_code;
    }
}
```
 

Use of _error_code_ with exception based error handling
------------------------------------------------------

Not everyone uses exceptions, and to be fair there has been a history or dubious standards in this area: 
* ```throw 99```
* ```catch (const char * err)```
* Or rely upon `catch (...)```
* Or rely on checking ```what()```

All the above are real world examples of code that went to production, or can be found in patent submissions, etc.

So far, all the above code would have worked if _error_code_ were an integral type so, however the identity of an _error_code_ is exploitable to provide good answers to a number of issues that can arise with committing to using exceptions and also maintains the good properties of exception handling.
For example: inheriting from one of the standard exception types such that it has the same global semantics of identity as _error_code_ is useful for the same reasons. There are quite a few techniques one can explore in this direction, but to pick a good example exception: class templates specialised on _error_code_ are very apt:

    // we can define a simple template parameterised upon the error_code value
    // this one has a base type and optional additional info
    // some book keeping is done to keep the data requirement to a minimum
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


Example: this allows us to define exceptions concisely based upon an error_code.

```
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
    catch (bar_err &e)
    {
      // or a typedef
    }
    catch (...)
    {
      // we don't get here
    }
```


And rather neatly we can avoid "false matches" caused by code handling caused exception types over eagerly. 
Having a unified set of identities allows callees to throw an exception, relying upon callers higher in the stack to make the decision on how to handle that status, and avoiding the need re-throw the exception. Even if re-thrown, the identity is of course preserved even if the stack at the point of re-throw is different from the originating thrower.

    try
    {
      // something that throws a typed_error<LibA::EPOR>
    }
    catch (typed_error<LibA::EBAR> &e)
    {
      // not caught
    }
    catch (std::runtime_error &e)
    {
      // typed_error<LibA::EPOR> is caught here, conveniently
    } 


TODO(PMM) - think about template instantiation.

There is one responsibility that is granted with the benefit of universality: since everyone could receive an error code, there may be a need to approach declaring _errode_code_ instances to some corporate standard of consistency. This may well mandate some kind of scheme perhaps based upon library, component, etc. to generate standard values.

    ```
    #define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str
    ```
    
and used thus:

```
    const char LibA::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");
```

which give us

```
    "GRP-FOO: Foo not reparable"
```
    
organisations can exploit this, and perhaps some other features conferring uniqueness  such as ```__FILE__```, ```__LINE__```


No Existential Forgery of _error_code_
--------------------------------------

So, what do is conveyed by "existential forgery"?
There are two types
The first is caused innocently enough by interfacing c++ client code a C style API which defins an enum for the return status type from an interface. This *forces* us to make a mapping some status and another - clearly this is a good place for incorrect logic to creep in, on the basis on the nature of the code. 
The second is caused by the problem that integral types are a built in type and have values that can be defined. Hence an undocumented API return value from a library can be "fixed" by simply performing remedial action for that value, trusting that value. This is by definition not proven. It can also be super time consuming to find this kind of problem in a large code base for a specific library - as the reverse lookup of who defined *that* 42 is very timeconsuming when it's been passed through APIs. The integral values cannot be made private.

```
case 42:
    // we need to handle this concurrency
    concurrency_action:
```    


For error_code, making the same _faux pas_ this is quite simply very hard to impossible, even with full access to the machinery [ citation needed] 

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

No solution is perfect, and this is no exception, however let's list the main concerns one woulc come up with and address them:

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
  - For this objection, absolutely no. Because these can be used opaquely for an identity there is NO WAY the system can enforce a rule that avoids a developer making a serious security error by displaying an error of unknown origin and hence *unknown sensitivity* to the end user. Instead it is essential to choose what should be shown to whatever user is present at the call site under question, and do that translation at that point.

* TODO out of process, dynamic loading etc. type concerns

Wrap up
-------

In summary, once the perhaps slighlty odd feeling of using  _error_code_ fades, we hope it is a technique that people will consider in composing their larger programs and when they design the error handling strategy up front.
Because you do do that, right? ;-)



[Google 2015, http://google.github.io/styleguide/cppguide.html ]

[Bloomberg 2015, https://github.com/bloomberg/bde/wiki/CodingStandards.pdf]

[Mozilla 2015, https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style#Error_handling]

[wikipedia 2015 https://en.wikipedia.org/wiki/HRESULT]

[Ruby 2015 http://ruby-doc.org/core-1.9.3/Symbol.html]

[Clojure 2015 http://clojure.org/data_structures#toc8]
