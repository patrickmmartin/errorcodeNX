/*
 * test_typederror.cpp
 *
 *  Created on: 3 Oct 2015
 *      Author: Patrick
 */

#include "catch/catch.hpp"
#include "error_id.hpp"

#include "fooerrors.h"

static bool dispatch_default_called = false;
static bool dispatch_foo_called = false;

template <error_id errtype> class ErrorDispatcher {
private:
  error_id _errtype;
  void dispatchError() { dispatch_default_called = true; };

public:
  ErrorDispatcher() : _errtype(errtype) {}

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
template <error_id x, typename xs> struct CheckList<ErrorList<x, xs> > {
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
  CHECK(switch_foo_called == true);

  K = FooErrors::EBAR;
  CheckList<ErrorsXY>()(K);

  K = FooErrors::EPOR;
  CheckList<ErrorsXY>()(K);
  CHECK(switch_default_pass_called == true);
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
  CHECK(switch_foo_called == true);

  K = FooErrors::EBAR;
  CheckList<ErrorsXYOnly>()(K);

  K = FooErrors::EPOR;
  CheckList<ErrorsXYOnly>()(K);
  CHECK(switch_default_fail_called == true);

  // illustrates statically mandating handlers for specified error codes
  // this cannot compile without an implementation of ErrorHandler<FooErrors::Z>
  // error: invalid use of incomplete type 'struct ErrorHandler<((const char*)(&
  // FooErrors::Z))>'
  //    K = FooErrors::Z;
  //    CheckList<ErrorsXYZ>()(K);
}
