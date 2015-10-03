/*
 * test_errorid.cpp
 *
 *  Created on: 30 Sep 2015
 *      Author: Patrick
 */

#include "errorcode.hpp"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstring>

#include "catch/catch.hpp"

#include "LibA.h"

namespace {

// this struct merely supplies a namespace for holding errors
struct FooErrors {
  static const char EFOO[];
  static const char EBAR[];
  static const char EPOR[];
};

// this is a hand-crafted error definition
const char FooErrors::EFOO[] = "GRP-FOO: Foo clobbered BAR on use";
// and these use the convenience macro
const char FooErrors::EBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char FooErrors::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

// we can define new unique exception instances
typedef typed_error<FooErrors::EFOO> foo_err;

typedef typed_error<FooErrors::EBAR> bar_err;

// or we can use the value directly
error_id const A = FooErrors::EFOO;
error_id const B = FooErrors::EBAR;

// the concept can be used directly for a basic unique error condition
struct N {
  static const char new_foo[];
  static const char new_bar[];
  static const char new_foo2[];
};

const char N::new_bar[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char N::new_foo[] = SCOPE_ERROR_UNIQUE("GRP", "FOO", "Foo not Bar");
const char N::new_foo2[] = SCOPE_ERROR_UNIQUE("GRP", "FOO", "Foo not Bar");

TEST_CASE("check macro works", "[errorcode]") {

  INFO(N::new_bar);
  INFO("equality of error strings generated the same way is possible");
  CHECK((strcmp(N::new_bar, FooErrors::EBAR) == 0));
  INFO("verify the output of the macro");
  CHECK((strcmp(N::new_bar, "GRP"
                            "-"
                            "FOO"
                            ": "
                            "Foo not Bar") == 0));

  INFO(N::new_bar);
  CHECK((N::new_bar != FooErrors::EBAR));
  INFO(N::new_foo2);
  CHECK((N::new_foo != N::new_foo2));

}

TEST_CASE("access values directly", "[errorcode]") {

  // the concept can be used directly for a basic unique error condition
  error_id raw_foo = FooErrors::EBAR;

  INFO("testing raw values");
  CHECK((raw_foo == FooErrors::EBAR));
  CHECK((raw_foo != FooErrors::EFOO));
}


TEST_CASE("check values write correctly", "[errorcode]") {


  char buf[1024] = {0};

  error_value raw_foo = NULL;

  INFO("testing raw values");
  std::ostringstream ostr;

  ostr << raw_foo;
  INFO(ostr.str());
  CHECK((ostr.str() == ""));

  snprintf(buf, sizeof(buf), "%s", raw_foo);
  INFO(buf);
  std::string bufstr = buf;
  CHECK((bufstr == buf));

  raw_foo = FooErrors::EBAR;
  ostr.str(""); ostr.clear();
  ostr << raw_foo;
  INFO(ostr.str());
  CHECK((ostr.str() == FooErrors::EBAR));

  snprintf(buf, sizeof(buf), "%s", raw_foo);
  bufstr = buf;
  INFO(bufstr);
  CHECK((bufstr == std::string(FooErrors::EBAR)));



  }



TEST_CASE("check returned values", "[errorcode]") {

  // the concept can be used directly for a basic unique error condition
  SECTION("testing raw values") {
    error_value err;
    err = LibA::return_me(0);
    INFO(err);
    CHECK((err != FooErrors::EBAR));
    CHECK((err != FooErrors::EFOO));
    CHECK((err == LibA::EFOO));

    err = LibA::return_me(1);
    INFO(err);
    CHECK((err != FooErrors::EBAR));
    CHECK((err != FooErrors::EFOO));
    CHECK((err == LibA::EBAR));

    err = LibA::return_me(-1);
    INFO(err);
    CHECK((err != FooErrors::EBAR));
    CHECK((err != FooErrors::EFOO));
  }
}


}
