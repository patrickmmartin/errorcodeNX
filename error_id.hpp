/*
 * error_id.hpp
 *
 *  Created on: 26 Aug 2015
 *      Author: Patrick
 */

#ifndef ERROR_ID_HPP_
#define ERROR_ID_HPP_

#include <string>
#include <iostream>

// ok, here's the big reveal...
// the rvalue
typedef const char *const error_id;

// and the lvalue
typedef const char *error_value;

/* Helper for concatenating argument in a standard
 * This is just a macro, and as such the semantics of the argument list
 * need to be maintained.
 * If different usage is required, define a new macro, or overwrite the
 * definition
 */
#define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SCOPE_ERROR_UNIQUE_FULL(grp, pkg, error_str)                                \
		__FILE__ ":" TOSTRING(__LINE__) " " grp "-" pkg ": " error_str " " __DATE__  __TIME__

#define SCOPE_ERROR_UNIQUE_SHORT(grp, pkg, error_str)                                \
       (__DATE__   __TIME__  __FILE__ ":" TOSTRING(__LINE__) " " grp "-" pkg ": " error_str ) + 19


// we can define a simple template parameterised upon an error_id
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

// And, of course we can define a minimal template parameterised upon an
// error_id
template <error_id errtype> class typed_error_lite : public std::exception {};

#endif /* ERROR_ID_HPP_ */
