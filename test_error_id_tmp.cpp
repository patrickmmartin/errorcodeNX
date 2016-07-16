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
  error_value _errtype;
  void dispatchError() { dispatch_default_called = true; };

public:
  ErrorDispatcher() : _errtype(errtype) {}

  void operator()() {
    // this member is provided by the specialisations
    dispatchError();
  };
};

template <> void ErrorDispatcher<FooErrors::eFOO>::dispatchError() {
  dispatch_foo_called = true;
}

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

  ErrorDispatcher<FooErrors::eFOO>()();
  CHECK(dispatch_foo_called);

  ErrorDispatcher<FooErrors::eBAR>()();
  CHECK(dispatch_default_called);
}

// switch statements are basically just a problem -they mandate constants
// so nothing here for a nice clean syntax of switch

TEST_CASE("demonstrate static dispatchers adding case", "[errorcode]") {

  // However we can use template specialisation, as demonstrated below

  dispatch_default_called = false;
  dispatch_foo_called = false;

  ErrorDispatcher<FooErrors::eFOO>()();
  CHECK(dispatch_foo_called);

  ErrorDispatcher<FooErrors::eBAR>()();
  CHECK(dispatch_default_called);
}

bool switch_default_pass_called = false;
bool switch_default_fail_called = false;
bool switch_foo_called = false;

/**
 * an error handler template, for later use
 */
template <error_id err_type> struct ErrorHandler;

/** specialisation for FooErrors::eFOO
 *
 */
template <> struct ErrorHandler<FooErrors::eFOO> {
  void operator()() { switch_foo_called = true; }
};

/** specialisation for FooErrors::BAR
 *
 */
template <> struct ErrorHandler<FooErrors::eBAR> {
  void operator()() {}
};

/**
 * typelist construction to support statically checked error handling
 */
struct PassFallThrough;
struct FailFallThrough;
template <error_id x, typename xs> struct ErrorList;

/** list checker template
 *
 */
template <typename T> struct CheckList {};

/**
 * a base case for the list checker that sets true
 */
template <> struct CheckList<PassFallThrough> {
  void operator()(error_id n, bool &handled) {
    switch_default_pass_called = true || (n) || handled;
  }
};

/**
 * a base case for the list checker thats sets false
 */
template <> struct CheckList<FailFallThrough> {
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
  typedef ErrorList<FooErrors::eFOO, ErrorList<FooErrors::eBAR, PassFallThrough> >
      ErrorsFooBar;

  // exercising with constants
  CheckList<ErrorsFooBar>()(FooErrors::eFOO);
  CheckList<ErrorsFooBar>()(FooErrors::eBAR);
  CheckList<ErrorsFooBar>()(FooErrors::ePOR);

  // exercising with variables
  error_value K = FooErrors::eFOO;
  CheckList<ErrorsFooBar>()(K);
  CHECK(switch_foo_called);

  K = FooErrors::eBAR;
  CheckList<ErrorsFooBar>()(K);

  K = FooErrors::ePOR;
  CheckList<ErrorsFooBar>()(K);
  CHECK(switch_default_pass_called);
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
  typedef ErrorList<FooErrors::eFOO,
		  	  ErrorList<FooErrors::eBAR,
			  	  FailFallThrough> >
      ErrorsFooBarOnly;

  // exercising with variables
  error_value K = FooErrors::eFOO;

  // exercising with variables
  K = FooErrors::eFOO;

  CheckList<ErrorsFooBarOnly>()(K);
  CHECK(switch_foo_called);

  K = FooErrors::eBAR;
  CheckList<ErrorsFooBarOnly>()(K);

  K = FooErrors::ePOR;
  CheckList<ErrorsFooBarOnly>()(K);
  CHECK(switch_default_fail_called);

  // illustrates statically mandating handlers for specified error codes
  // this cannot compile without an implementation of ErrorHandler<FooErrors::ePOR>
  // error: invalid use of incomplete type 'struct ErrorHandler<((const char*)(& FooErrors::ePOR))>'

  //  typedef ErrorList<FooErrors::eFOO,
  //               ErrorList<FooErrors::eBAR,
  //                    ErrorList<FooErrors::ePOR,
  //                         FailType> > >
  //      ErrorsFooBarPorRequired;

  //  K = FooErrors::ePOR;
  //  CheckList<ErrorsFooBarPorRequired>()(K);
}
