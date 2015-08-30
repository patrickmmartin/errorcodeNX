# errorcodeNX

Error codes
===========

Overview
--------

Errorcodes have always caused controversy, one way or the other.

* People will take the time to laugh about ERROR_SUCCESS (the value is 0 by the way)
* developers over time will erode any former integrity there ever may have been in a large code base by inventing deathless codes such as -99 "because no-one else could choose that value"
* even companies with a large, enforced schemes on their products will have issues
  ** ORA-06502: PL/SQL: numeric or value error?
  ** ORA-00942: Table or view does not exist
  ** -148 THE SOURCE TABLE OR TABLESPACE source-name CANNOT BE ALTERED, REASON reason-code
  ** 0xC0000005
  
Reviewing these and other schemes, some themes emerge:
* If using integers to encode - maintaining a completely reliable registry of errors is a hard problem.
* Exceptions as flow control, where not a _priori_ ruled out through language support / language module compatibility is often a matter of taste / religion
* Mechanisms such a central registry and an approach of "register your errors at runtime" are run-time heavy and have a non-trivial cost

What I want to persuade you of here is there is a lightweight approach that has some merit and useful properties, which can accomdate a number of styles.

It won't fit all the bills, but for binaries built statically from large source bases, it has some uses.

 
 
TODO(PMM) think about these
=========
thread local installation of error handler
------------------------------------------

TMP with error code value?
--------------------------

"conceptual stack" (when thrown) (pluggable)
--------------------------------------------
same as chainability ?
----------------------
note that the current assumption is the base type is not an arbitrary template

"actual stack" (pluggable)
--------------------------
there are many examples of stack tracing 


