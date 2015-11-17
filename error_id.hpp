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
#include <stdexcept>

// ok, here's the big reveal...
// the rvalue
typedef char const error_id[];

// and the lvalue
typedef char const* error_value;

/* Helper for concatenating argument in a standard
 * This is just a macro, and as such the semantics of the argument list
 * need to be maintained.
 * If different usage is required, define a new macro, or overwrite the
 * definition
 */
#define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str

// Organisations can exploit other preprocessor
// features to ensure uniqueness of output

#define TOSTRING(x) #x

#define SCOPE_ERROR_LOCATION(grp, pkg, error_str) \
        __FILE__ ":" TOSTRING(__LINE__) " " grp "-" pkg ": " error_str " "

// we can define a simple template parameterised upon the error_id value
template <error_id errtype> class typed_error_lite : public std::exception {};

// or we can go a little further and allow for some additional information
// this one has a base type and additional info
template <error_id errtype> class typed_error : public std::runtime_error {
public:
  // be very careful to ensure that what is given a NBTS
  typed_error(const char* what = errtype): std::runtime_error(what) {}

  const char *type() const { return errtype; }
  operator const char *() { return errtype; }
};

#endif /* ERROR_ID_HPP_ */
