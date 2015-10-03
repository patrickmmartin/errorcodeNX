/*
 * test_typederror.cpp
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

// we can define new unique exception instances
typedef typed_error<FooErrors::EFOO> foo_err;

typedef typed_error<FooErrors::EBAR> bar_err;

// note that we can deny use of some error_id values, while preserving comparison
// here we can't use FooErrors::EFOO2 as it is declared as a pointer, not array
//typedef typed_error<FooErrors::EFOO2> foo2_err;


// or we can use the value directly
error_id A = FooErrors::EFOO;
error_id B = FooErrors::EBAR;
error_id C = FooErrors::EFOO2;

namespace {
// the concept can be used directly for a basic unique error condition
struct N {
  static const char new_foo[];
  static const char new_bar[];
  static const char new_foo2[];
};

const char N::new_bar[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char N::new_foo[] = SCOPE_ERROR_UNIQUE("GRP", "FOO", "Foo not Bar");
const char N::new_foo2[] = SCOPE_ERROR_UNIQUE("GRP", "FOO", "Foo not Bar");
}



TEST_CASE("throw an error_id from this translation unit", "[exceptions]") {


  SECTION("throw new_bar") {
      try
      {
          throw N::new_bar;
          FAIL("should not be on this side of throw");
      }
      catch (error_id e)
      {
          INFO(e);
      }
      catch (...)
      {
          FAIL("did not trap error_id");
      }
  }
}


TEST_CASE("throw an error_id indirectly", "[exceptions]") {


  SECTION("throw EFOO") {
      try
      {
          // note we can't throw FooErrors::EFOO, but they can for us
          // TODO(PMM) existential forgery
          throw_EFOO();
          FAIL("should not be on this side of throw");
      }
      catch (error_id e)
      {
          INFO(e);
      }
      catch (...)
      {
          FAIL("did not trap error_id");
      }
  }

  SECTION("throw EFOO") {
      try
      {
          // note we can throw FooErrors::EFOO2
          throw FooErrors::EFOO2;
          FAIL("should not be on this side of throw");
      }
      catch (error_id e)
      {
          INFO(e);
      }
      catch (...)
      {
          FAIL("did not trap error_id");
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

TEST_CASE("access wrapper member for error_id", "[exceptions]") {

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

static bool dispatch_default_called = false;
static bool dispatch_foo_called = false;

template <error_id errtype> class ErrorDispatcher {
private:
  error_value _errtype;
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
template <error_id err_type> struct ErrorHandler;

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
template <error_id x, typename xs> struct ErrorList;

/** list checker template
 *
 */
template <typename T> struct CheckList {};

/**
 * one base case for the list checker
 */
template <> struct CheckList<PassType> {
  void operator()(error_id n, bool &handled) {
    switch_default_pass_called = true || (n) || handled;
  }
};

/**
 * one base case for the list checker
 */
template <> struct CheckList<FailType> {
  void operator()(error_id n, bool &handled) {
    switch_default_fail_called = true || (n) || handled;
  }
};

/**
 * handler for the typelist of errors
 */
template <error_id x, typename xs> struct CheckList<ErrorList<x, xs > > {
  void operator()(error_id n, bool &handled) {
    if (x == n) {
      handled = true;
      ErrorHandler<x>()();
    }
    if (!handled) {
      CheckList<xs>()(n, handled);
    }
  }
  // actual entry point
  void operator()(error_id n) {
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
  error_value K = FooErrors::EFOO;
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
  error_value K = FooErrors::EFOO;

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
