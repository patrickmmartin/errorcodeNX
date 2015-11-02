/*
 * LibA.h
 *
 *  Created on: 31 Aug 2015
 *      Author: patrick
 */

#ifndef LIBA_H_
#define LIBA_H_

#include <stddef.h>

#include "error_id.hpp"

class LibA {

public:
  static const char eFOO[];
  static const char eBAR[];

  static error_value return_me(int input);

  static typed_error<eFOO> get_foo(const char *message);

  static void foo_me(const char *message);

  static void bar_me(const char *message);

  static void suprise_me(const char *message);

private:
  static const char ePOR[];
};

#endif /* LIBA_H_ */
