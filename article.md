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
* gtrade_px returning 2, checked function - returns 2
* a function returns -9999 as status that is for a system essentially not in an error state. [The hilarious thing is, there are multiple different developers with their ears prickling at this point]

Typically ints / enums are used.
But...
Composability is effectively broken, either we don't understand the value returned in and of itself, or we're forced to translate/ coerce it, so now the *consumer* doesn't understand it.

Whereas for error_code

Utility is completely global
So values can be passed through opaquely
[Example]

Error conditions can be composed manually with little effort and much code hygiene
[Example]

This is new...
Error conditions can be composed dynamically
[Example]

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

[Examples]

Now for the bad news...

No switch (but ya ain't gonna need it)
Can't return additional info. Well, by design most use cases do not make sense for a pure error code, this is where the python guys start laughing
      (info, error) = attempt_the_thing()

One missing use case in common with the integral values is the classic FILE_NOT_FOUND TABLE OR VIEW MISSING (but which?)
If you really want to be helpful, create a wrapper for the error_code, or use typed_error

Should these be translated for the user?
Absolutely no. Because these can be used opaquely there is NO WAY the system can enforce avoiding making a serious security error by displaying an error of unknown origin and hence sensitivity to the end user
Instead choose what you want to be shown, and show that.
