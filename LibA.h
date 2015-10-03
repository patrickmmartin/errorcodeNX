/*
 * LibA.h
 *
 *  Created on: 31 Aug 2015
 *      Author: patrick
 */

#ifndef LIBA_H_
#define LIBA_H_

#include <stddef.h>

#include "errorcode.hpp"

class LibA {

public:
  static const char EFOO[];
  static const char EBAR[];

  static error_value return_me(int input);

  static typed_error<EFOO> get_foo(const char *message = NULL);

  static void foo_me(const char *message = NULL);

  static void bar_me(const char *message = NULL);

  static void suprise_me(const char *message = NULL);

private:
  static const char EPOR[];
};

#endif /* LIBA_H_ */
