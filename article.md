Defining and using a symbol analogue for error reporting in c++
===============================================================

Overview
--------

High quality software requires a well defined and striaghtforward error handling strategy to allow a system to protect or check it's invariants in the face of untested inputs or configurations. There are many positions take on how to achieve this see [Google 2015], [Bloomberg 2015] [Mozilla 2015] [Wikipedia 2015], How to achieve this is contentious and the strategies for implementing it in a large system are contentious even in C21, it seems. [Google coding guidelines, others]
However, error handing is still important in general and especially so for C/C++ [citation needed]
In this article we will present an approach, which we'll call ```error_code``` which can be used an identity concept to ensure when a specific course of action is desired, the state reported by a failed API can be unambiguously recognised at arbitrarily remote call sites.

Review of c++ and C approaches
-----------------------------

A very common style in C is using ```enum``` or ```int``` TODO citations, HRESULT   . But this has significant issues, where interfaces are composed that have defined their own subsets of return codes, or HRESULT values need to be registered and reported.

In the case that a ```int``` is used, without a strictly enforced and maintained global registry of values, a function that needs to consume multiple interfaces and report errors encountered in its processing will have to inspect the return codes and itself define yet another set of error values. This leads to a proliferation of interfaces. However maintaining a global registry over a large source base has typically had mixed results.   
The situation where an ```enum``` is used is even more problematic as in c++ handling these mismatching interfaces requires a switch statement, with the risk of mismapping of new return codes introduced into lib_one_err_t.

For example, here is a simple constructed example in c++:

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
} 
```

Note that subsequently introduced new error states in ```lib_one_err_t``` will potentially require updates in all places where values of this ```enum``` are examined. The net effect is that interfaces must handle all returned statuses conservatively, which certainly results in less sophisticated program behaviour, which is inefficient, and further opens up the possibility that there is an error in all this additional code.  

For the proposed ```error_code``` this is not an issue.

The value is unique in the process, and as such can be used as an _identity_ concept, whose values can be passed through opaquely from any callee to any caller. Caller and callee can be separated by any number of layers of calls and yet the return values can be passed transparently back to a caller, allowing for less mandatory handling, resulting in less effort and less opportunity for erroneous handling. 

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

Example: error conditions can now be composed dynamically .

```
error_code ret;
for (test_step : test_steps)
{
 if (!ret = test_step(args))
 {
  log << "raised error [" << ret << "] in test step " << test_step;
  return error_code;
 }
}
 ```
 
There is no limit! [Caveat needed]

What is this magical type?

```
typedef const char * error_code;
```

An ```error_code``` value has a number of good properties.
* Globally registered, the linker handles it [standard section, dynamic loading etc.] [If patching your binary is your thing, then enjoy, but please aware what implications that process had]
* Can be used as an opaque value
* Yet if a string constant, then inherently it supports printing for logging purposses.

For the brave souls relying upon exceptions, there is a whole raft of controversies/antipatterns. I won't do more than touch upon those, but the gist is 
Don't do this:
* _throw 99_
* Or rely upon catch (...)
* Or always throw catch std::exception and check .what()

Extending an exception type such that it has the same global semantics of identity as error_code is useful for the same reasons. Templates specialised on error_code are very apt for this:

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

Example: this allows us to define exceptions concisely based upon and error_code.

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

And rather neatly we can avoid "false positives" in handling exceptions, allowing callees to throw an exception, relying upon callers higher in the stack to make the decision on how to handle that status.

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


The faustian bargain that is universality: since everyone could receive an error code, they may need to be handled to some level of consistency. This may well mandate some kind of scheme for declaring error_code - perhaps based upon library, component, etc. to generate standard values.

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
    
organisations can exploit this, or other tricks, such as FILE, LINE

So, what did I mean by "existential forgery"?
In c++ defining an enum for the return type from error interface *forces* us to do this 

hence a lot of code ported from C suffers from requiring analysis and changes (or you don't port it, and pass the problem on to the caller / future maintainers )

for error_code, this is quite simply either very hard to impossible, even with full access to the machinery [ citation needed]

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



Now for the bad news...

* No `switch` (but ya ain't gonna need it so much because errors will in the main case only _need to be thrown and consumed_ )
* Can't return additional info 
  - Well, by design most use cases do not make sense for a pure error code,
  - let us pause as this is where the python guys start laughing
      ```(info, error) = attempt_the_thing()```

    * One missing use case in common with the integral values is the classic FILE_NOT_FOUND TABLE OR VIEW MISSING (but which?) If it is useful to attach that parameter, then it seems reasonable that the consumer will need to request a pair<error_code, more_info>, use out parameters or devise some other scheme to compose the required data.

* Should these be translated for the user?
  - Absolutely no. Because these can be used opaquely there is NO WAY the system can enforce avoiding making a serious security error by displaying an error of unknown origin and hence sensitivity to the end user
Instead choose what you want to be shown only at the appropriate call site, and do that then.


[Google 2015, http://google.github.io/styleguide/cppguide.html ]
[Bloomberg 2015, https://github.com/bloomberg/bde/wiki/CodingStandards.pdf]
[Mozilla 2015, https://developer.mozilla.org/en-US/docs/Mozilla/Developer_guide/Coding_Style#Error_handling]
[wikipedia 2015 https://en.wikipedia.org/wiki/HRESULT]
