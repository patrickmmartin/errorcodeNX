Point to cover
* Globally registered, linker handles it [standard section, dynamic loading etc.] [If patching your binary is your thing, then enjoy, but please aware what implications that process had]
* Can (should) be used as an opaque value
* Intentional collisions very hard
* Accidental collisions exceptionally hard to impossible [depends upon definition?] 
* Lots of good properties
* Contrast with int, enum
* maybe introduces some new ones (but the "better kind of problem"?)
* "portability"
 - 'twixt compilers
 - 'twixt processes
 - 'twixt processes of different builds?
* for all the above, it is incumbent upon the code passing error_code between processes to implement some sensible marshalling and registration - the strings and the addresses are no use


$PUNCHY_TITLE

Error handling is contentious and the strategies for implementing it in a large system are contentious, even in C21, it seems. [Google coding guidelines, others]
However, error handing is still important in general and especially so for C/C++ [citation needed]

[Various horror stories]
* Prophet web client, returning zero for 404
* a function returns -9999 as status that is for a system essentially not in an error state. [The hilarious thing is, I know there are multiple different developers with their ears prickling at this point]

Typically ints / enums are used.
But...
Composability is effectively broken, either we don't understand the value returned in and of itself, or we're forced to translate/ coerce it, so now the *consumer* doesn't understand it.

Whereas for error_code

Utility is completely global
So values can be passed through opaquely
[Example]

Error conditions can be composed manually with little effort and much code hygiene

    error_code ret;
    if (!ret = in())
    {
      if (!ret = a_galaxy())
         {
             if (!ret = far_far_away())
             {
                  awaken_force();
             }
         }
    }
        
    if (ret)
    {
         // continuable errors here
         if (ret != FORD_BROKEN_ANKLE) && (ret != FORD_TRANSPORTATION_INCIDENT)
         {
             print_local_failure(ret);
             return ret;
         }
         else
         {
             update_tmblr(ret); // FOR TEH LULZ
         }
    
    }

This is new...
Error conditions can be composed dynamically
[Example]


    error_code ret;
    for (test_step : test_steps)
    {
     if (!ret = test_step(args))
     {
      log << "raised error [" << ret << "] in test step " << test_step;
      return error_code;
     }
    }
    
There is no limit! [Caveat needed]

What is this magical type?

For the brave souls relying upon exceptions, there is a whole raft of controversies/antipatterns. I won't do more than touch upon those, but the gist is 
Don't do this:
* _throw 99_
* Or rely upon catch (...)
* Or always throw catch std::exception and check .what()

IMHO People in systems supplying only a singly rooted object hierarchy for exceptions have made a pretty good job of making that work for them.

So... extending this so that exceptions have the same global semantics, allowing the same properties of error_code to shine through would be good.
Templates specialised on error_code are very apt

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

this allows:

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


and what is rather neat:

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


The faustian bargain that is universality: since everyone could receive an error code, they may need to be handled to some level of consistency.


    #define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str

and used thus:

    const char LibA::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");
which give us

    "GRP-FOO: Foo not reparable"
    
organisations can exploit this, or other tricks, such as FILE, LINE

So, what did I mean by "existential forgery"?

this is a thing in Java: clearly too much of this will prevent a nice clean unwind 

    void launderThrowable ( final Throwable ex )
    {
        if ( ex instanceof ExecutionException )
        {
            Throwable cause = ex.getCause( );
         
            if ( cause instanceof RuntimeException )
            {
                // Do not handle RuntimeExceptions
                throw cause;
            }
        
            if ( cause instanceof MyException )
            {
                // Intelligent handling of MyException
            }
            ...
        }
     }
[from Java Concurrency in Action]

existential forging is "liberating" a private numerical constant from its shackles in your callee's code base and re-using it

example: 
    
    if (err == -9999) /* sigh. */

similarly, in c++ defining an enum for your error handling *forces* us to do this


    lib_one_err_t func1(...)
        
    lib_two_err_t func2(
        
    lib_one_err_t ret1= func1(...);
        
    if (ret1)
    {
        
       switch (ret1)
       {
          case ONE_LIB_ERR_DB:
              return TWO_LIB_ERR_DB;
          default: // now what?
              return TWO_LIB_UNEXPECTED; // some people like _UNKNOWN - they know who they are.
                
       }
        
    }
    

or we could take a chance and cast it to int ...

neither of these are good IMHO

hence a lot of code ported from C suffers from requiring analysis and changes (or you don't port it, and pass the problem on to the caller / future maintainers )

anyway enough with the bonfire of straw men

for error_code, this is quite simply not possible, even with full access to the machinery [standard citation needed]

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

* No switch (but ya ain't gonna need it)
* Can't return additional info.
  - Well, by design most use cases do not make sense for a pure error code,
  - let us pause as this is where the python guys start laughing
      (info, error) = attempt_the_thing()

* One missing use case in common with the integral values is the classic FILE_NOT_FOUND TABLE OR VIEW MISSING (but which?)
  - If you really want to be helpful, create a wrapper for the error_code, or use typed_error

* Should these be translated for the user?
  - Absolutely no. Because these can be used opaquely there is NO WAY the system can enforce avoiding making a serious security error by displaying an error of unknown origin and hence sensitivity to the end user
Instead choose what you want to be shown, and show that.
