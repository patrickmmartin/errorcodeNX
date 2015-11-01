/*
 * LibA.cpp
 *
 *  Created on: 31 Aug 2015
 *      Author: patrick
 */

#include "LibA.h"

#include "error_id.hpp"

const char LibA::eFOO[] = SCOPE_ERROR("GRP", "FOO", "Foo clobbered BAR on use");
const char LibA::eBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char LibA::ePOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

error_value LibA::return_me(int input) {
  switch (input) {
  case 0:
    return LibA::eFOO;

  case 1:
    return LibA::eBAR;

  default:
    return LibA::ePOR;
  }
}

typed_error<LibA::eFOO> LibA::get_foo(const char *message) {
  return typed_error<LibA::eFOO>(message);
}

void LibA::foo_me(const char *message) {
  throw typed_error<LibA::eFOO>(message);
}

void LibA::bar_me(const char *message) {
  throw typed_error<LibA::eBAR>(message);
}

void LibA::suprise_me(const char *message) {
  throw typed_error<LibA::ePOR>(message);
}
