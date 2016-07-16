/*
 * except_id.hpp
 *
 *  Created on: 16 Jul 2016
 *      Author: patrick
 */

#ifndef EXCEPT_ID_HPP_
#define EXCEPT_ID_HPP_

#include <stdexcept>

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




#endif /* EXCEPT_ID_HPP_ */
