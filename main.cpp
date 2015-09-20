/*
 * main.cpp
 *
 *  Created on: 26 Aug 2015
 *  Author: Patrick Martin
 */

#include "errorcode.hpp"

#include <iostream>
#include <sstream>
#include <stdio.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch/catch.hpp"

#include "LibA.h"

// this struct merely supplies a namespace for holding errors
struct FooErrors {
public:
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
error_code const A = FooErrors::EFOO;
error_code const B = FooErrors::EBAR;

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
  error_code raw_foo = FooErrors::EBAR;

  INFO("testing raw values");
  CHECK((raw_foo == FooErrors::EBAR));
  CHECK((raw_foo != FooErrors::EFOO));
}


TEST_CASE("check values write correctly", "[errorcode]") {


  char buf[1024] = {0};

  error_code raw_foo = NULL;

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



TEST_CASE("throw an error_code directly", "[exceptions]") {


  SECTION("throw EFOO") {
      try
      {
         throw FooErrors::EFOO;
      }
      catch (error_code e)
      {
          INFO(e);
      }
      catch (...)
      {
          FAIL("did not trap error_code");
      }
  }
}



TEST_CASE("constructing wrapper for error", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  const char *msg = "foo is not a bar - thirst ensues";
  foo_err err(msg);
  foo_err err_blank;

  SECTION("testing no-args construction") {
    INFO("testing comparison for wrapped values");
    CHECK((err_blank.type() == FooErrors::EFOO));
    CHECK((err_blank.type() != FooErrors::EBAR));
    INFO(err_blank.what());
    CHECK((strcmp(err_blank.what(), FooErrors::EFOO) == 0));
  }

  SECTION("testing construction with additional string") {
    INFO("testing wrapped values");
    CHECK((err.type() == FooErrors::EFOO));
    CHECK((err.type() != FooErrors::EBAR));
    INFO(err.what());
    CHECK((err.what() == std::string(FooErrors::EFOO) + ". " + msg));
  }
}

TEST_CASE("access wrapper member for error_code", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  INFO("testing comparison wrapped values");
  CHECK((err.type() == FooErrors::EFOO));
  CHECK((err.type() != FooErrors::EBAR));
}

TEST_CASE("access wrapper operator for error overload", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  CHECK((err == FooErrors::EFOO));
  CHECK((err != FooErrors::EBAR));
}

TEST_CASE("ensure throwing exception instances works", "[exceptions]") {

  // another possible use of the template class is to have highly specific
  // exception handling
  SECTION("throw and catch typed_error<FooErrors::EFOO>") {
    try {
      throw foo_err("foo != bar");
    } catch (typed_error<FooErrors::EFOO> &e) {
      INFO("caught in foo_err handler");
    } catch (bar_err &e) {
      FAIL("Fell through to bar_err handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("throw and catch typed_error<FooErrors::EBAR>") {
    try {
      throw bar_err("bazong not convertible to bar");
    } catch (typed_error<FooErrors::EFOO> &e) {
      FAIL("Caught in foo_err handler");
    } catch (bar_err &e) {
      INFO("in bar_err handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("throw and catch std::exception") {
    try {
      throw bar_err("bazong not convertible to bar");
    } catch (std::exception &e) {
      INFO("in std::exception handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("existential forgery of typed_error is not possible") {
    try {
      CHECK((N::new_bar != FooErrors::EBAR));
      throw typed_error<N::new_bar>("bazong not convertible to bar");
    } catch (typed_error<N::new_bar> &e) {
      INFO("in std::exception handler");
    } catch (typed_error<FooErrors::EBAR> &e) {
      FAIL("in typed_error<FooErrors::EBAR> handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

}

bool dispatch_default_called = false;
bool dispatch_foo_called = false;

template <error_code errtype> class ErrorDispatcher {
private:
  error_code _errtype;
  void dispatchError() { dispatch_default_called = true; };

public:
  ErrorDispatcher() { _errtype = errtype; }

  void operator()() {
    // this member is provided by the specialisations
    dispatchError();
  };
};

TEST_CASE("demonstrate static dispatchers", "[errorcode]") {

  // switch statements are basically just a problem -they mandate constants
  // so nothing here for a nice clean syntax of switch

  // However we can used template specialisation
  // this caters for having set up a set of handlers in scope prepared in
  // advance
  // there is a single site to inherit all that handling
  // this all works nicely because the code paths are defined at compile time

  dispatch_default_called = false;
  dispatch_foo_called = false;

  ErrorDispatcher<FooErrors::EFOO>()();
  CHECK(dispatch_foo_called);

  ErrorDispatcher<FooErrors::EBAR>()();
  CHECK(dispatch_default_called);
}

template <> void ErrorDispatcher<FooErrors::EFOO>::dispatchError() {
  dispatch_foo_called = true;
}

// switch statements are basically just a problem -they mandate constants
// so nothing here for a nice clean syntax of switch

TEST_CASE("demonstrate static dispatchers adding case", "[errorcode]") {

  // However we can use template specialisation, as demonstrated below

  dispatch_default_called = false;
  dispatch_foo_called = false;

  ErrorDispatcher<FooErrors::EFOO>()();
  CHECK(dispatch_foo_called);

  ErrorDispatcher<FooErrors::EBAR>()();
  CHECK(dispatch_default_called);
}

bool switch_default_pass_called = false;
bool switch_default_fail_called = false;
bool switch_foo_called = false;

/**
 * an error handler template, for later use
 */
template <error_code err_type> struct ErrorHandler;

/** specialisation for FooErrors::EFOO
 *
 */
template <> struct ErrorHandler<FooErrors::EFOO> {
  void operator()() { switch_foo_called = true; }
};

/** specialisation for FooErrors::BAR
 *
 */
template <> struct ErrorHandler<FooErrors::EBAR> {
  void operator()() {}
};

/**
 * typelist construction to support statically checked error handling
 */
struct PassType;
struct FailType;
template <error_code x, typename xs> struct ErrorList;

/** list checker template
 *
 */
template <typename T> struct CheckList {};

/**
 * one base case for the list checker
 */
template <> struct CheckList<PassType> {
  void operator()(error_code n, bool &handled) {
    switch_default_pass_called = true || (n) || handled;
  }
};

/**
 * one base case for the list checker
 */
template <> struct CheckList<FailType> {
  void operator()(error_code n, bool &handled) {
    switch_default_fail_called = true || (n) || handled;
  }
};

/**
 * handler for the typelist of errors
 */
template <error_code x, typename xs> struct CheckList<ErrorList<x, xs > > {
  void operator()(error_code n, bool &handled) {
    if (x == n) {
      handled = true;
      ErrorHandler<x>()();
    }
    if (!handled) {
      CheckList<xs>()(n, handled);
    }
  }
  // actual entry point
  void operator()(error_code n) {
    bool handled = false;
    operator()(n, handled);
  }
};

TEST_CASE("demonstrate error typelist handlers with fallthrough pass",
          "[errorcode]") {

  switch_default_pass_called = false;
  switch_default_fail_called = false;
  switch_foo_called = false;

  /**
   * list to define a list with required handlers errors
   * passed in with fall-through
   * X, Y
   */
  typedef ErrorList<FooErrors::EFOO, ErrorList<FooErrors::EBAR, PassType> >
      ErrorsXY;

  // exercising with constants
  CheckList<ErrorsXY>()(FooErrors::EFOO);
  CheckList<ErrorsXY>()(FooErrors::EBAR);
  CheckList<ErrorsXY>()(FooErrors::EPOR);

  // exercising with variables
  error_code K = FooErrors::EFOO;
  CheckList<ErrorsXY>()(K);
  CHECK((switch_foo_called == true));

  K = FooErrors::EBAR;
  CheckList<ErrorsXY>()(K);

  K = FooErrors::EPOR;
  CheckList<ErrorsXY>()(K);
  CHECK((switch_default_pass_called == true));
}

TEST_CASE("demonstrate typelist handlers with fallthrough fail",
          "[errorcode]") {

  switch_default_pass_called = false;
  switch_default_fail_called = false;
  switch_foo_called = false;

  /**
   * list to define a list with required handlers errors
   * passed and fall-through is an error
   * X, Y
   */
  typedef ErrorList<FooErrors::EFOO, ErrorList<FooErrors::EBAR, FailType> >
      ErrorsXYOnly;

  // exercising with variables
  error_code K = FooErrors::EFOO;

  // exercising with variables
  K = FooErrors::EFOO;

  CheckList<ErrorsXYOnly>()(K);
  CHECK((switch_foo_called == true));

  K = FooErrors::EBAR;
  CheckList<ErrorsXYOnly>()(K);

  K = FooErrors::EPOR;
  CheckList<ErrorsXYOnly>()(K);
  CHECK((switch_default_fail_called == true));

  // illustrates statically mandating handlers for specified error codes
  // this cannot compile without an implementation of ErrorHandler<FooErrors::Z>
  // error: invalid use of incomplete type 'struct ErrorHandler<((const char*)(&
  // FooErrors::Z))>'
  //    K = FooErrors::Z;
  //    CheckList<ErrorsXYZ>()(K);
}

TEST_CASE("check returned values", "[errorcode]") {

  // the concept can be used directly for a basic unique error condition
  SECTION("testing raw values") {
    error_code err;
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

TEST_CASE("checking returned wrapper for error", "[exceptions]") {

  const char *msg = "foo is not a bar - thirst ensues";
  typed_error<LibA::EFOO> err = LibA::get_foo(msg);
  typed_error<LibA::EFOO> err_blank = LibA::get_foo();

  SECTION("testing no-args construction") {
    INFO("testing comparison for wrapped values");
    CHECK((err_blank.type() == LibA::EFOO));
    CHECK((err_blank.type() != LibA::EBAR));
    INFO(err_blank.what());
    CHECK((strcmp(err_blank.what(), LibA::EFOO) == 0));
  }

  SECTION("testing construction with additional string") {
    INFO("testing wrapped values");
    CHECK((err.type() == LibA::EFOO));
    CHECK((err.type() != LibA::EBAR));
    INFO(err.what());
    CHECK((err.what() == std::string(LibA::EFOO) + ". " + msg));
  }
}

TEST_CASE("ensure handling thrown exception instances works", "[exceptions]") {

  // check specific exception handling
  SECTION("throw and catch typed_error<LibA::EFOO>") {
    try {
      LibA::foo_me();
    } catch (typed_error<LibA::EFOO> &e) {
      INFO("caught in LibA::EFOO handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("throw and catch typed_error<LibA::EBAR>") {
    try {
      LibA::bar_me();
    } catch (typed_error<LibA::EBAR> &e) {
      INFO("Caught in LibA::EFOO handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }
  SECTION("throw and catch unknown") {
    try {
      LibA::suprise_me();
    } catch (typed_error<LibA::EBAR> &e) {
      FAIL("Caught in LibA::EBAR handler");
    } catch (std::exception &e) {
      INFO(e.what());
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }
}
