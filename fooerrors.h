/*
 * fooerrors.h
 *
 *  Created on: 3 Oct 2015
 *      Author: patrick
 */

#ifndef FOOERRORS_H_
#define FOOERRORS_H_

#include "error_id.hpp"

namespace FooErrors {
  extern error_id eFOO;
  extern error_id eBAR;
  extern error_id ePOR;
  extern char const *eFOO2;
}

void throw_eFOO();

#endif /* FOOERRORS_H_ */
