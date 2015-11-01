/*
 * test_errorid.cpp
 *
 *  Created on: 30 Sep 2015
 *      Author: Patrick
 */

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstring>

#include "catch/catch.hpp"
#include "error_id.hpp"

#include "LibA.h"

#include "fooerrors.h"

namespace {
// the concept can be used directly for a basic unique error condition
struct N {
  static const char new_foo[];
  static const char new_bar[];
  static const char new_foo2[];
  static error_id new_foo3;
};


const char N::new_bar[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char N::new_foo[] = SCOPE_ERROR_UNIQUE_FULL("GRP", "FOO", "Foo not Bar");
const char N::new_foo2[] = SCOPE_ERROR_UNIQUE_FULL("GRP", "FOO", "Foo not Bar");
error_id N::new_foo3 = SCOPE_ERROR_UNIQUE_SHORT("GRP", "FOO", "Foo not Bar");


}


TEST_CASE("check macro works", "[errorcode]") {

  INFO(N::new_bar);
  INFO("equality of error strings generated the same way is possible");
  CHECK(!strcmp(N::new_bar, FooErrors::eBAR));
  INFO("verify the output of the macro");
  CHECK(!strcmp(N::new_bar, "GRP"
                           "-"
                           "FOO"
                           ": "
                           "Foo not Bar"));

  INFO(N::new_bar);
  CHECK((N::new_bar != FooErrors::eBAR));
  INFO("checking the simple macro");
  INFO(N::new_foo2);
  CHECK((N::new_foo != N::new_foo2));

  INFO("checking the macro trick works");
  INFO(N::new_foo3);

 // this symbol seems to be most widely defined
#ifdef _WIN32
  CHECK(!strcmp(N::new_foo3, "..\\test_error_id.cpp:33 GRP-FOO: Foo not Bar"));
#else
  CHECK(!strcmp(N::new_foo3, "../test_error_id.cpp:33 GRP-FOO: Foo not Bar"));
#endif

}

TEST_CASE("access values directly", "[errorcode]") {

  // the concept can be used directly for a basic unique error condition
  error_id raw_foo = FooErrors::eBAR;

  INFO("testing raw values");
  CHECK((raw_foo == FooErrors::eBAR));
  CHECK((raw_foo != FooErrors::eFOO));
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

  raw_foo = FooErrors::eBAR;
  ostr.str("");
  ostr.clear();
  ostr << raw_foo;
  INFO(ostr.str());
  CHECK((ostr.str() == FooErrors::eBAR));

  snprintf(buf, sizeof(buf), "%s", raw_foo);
  bufstr = buf;
  INFO(bufstr);
  CHECK((bufstr == std::string(FooErrors::eBAR)));
}

TEST_CASE("check returned values", "[errorcode]") {

  // the concept can be used directly for a basic unique error condition
  SECTION("testing raw values") {
    error_value err;
    err = LibA::return_me(0);
    INFO(err);
    CHECK((err != FooErrors::eBAR));
    CHECK((err != FooErrors::eFOO));
    CHECK((err == LibA::eFOO));

    err = LibA::return_me(1);
    INFO(err);
    CHECK((err != FooErrors::eBAR));
    CHECK((err != FooErrors::eFOO));
    CHECK((err == LibA::eBAR));

    err = LibA::return_me(-1);
    INFO(err);
    CHECK((err != FooErrors::eBAR));
    CHECK((err != FooErrors::eFOO));
  }
}
