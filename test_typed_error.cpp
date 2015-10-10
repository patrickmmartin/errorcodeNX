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

// note that we can deny use of some error_id values, while preserving
// comparison
// here we can't use FooErrors::EFOO2 as it is declared as a pointer, not array
// typedef typed_error<FooErrors::EFOO2> foo2_err;

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
    try {
      throw N::new_bar;
      FAIL("should not be on this side of throw");
    } catch (error_id e) {
      INFO(e);
    } catch (...) {
      FAIL("did not trap error_id");
    }
  }
}

TEST_CASE("throw an error_id indirectly", "[exceptions]") {

  SECTION("throw EFOO") {
    try {
      // note we can't throw FooErrors::EFOO, but they can for us
      // TODO(PMM) existential forgery
      throw_EFOO();
      FAIL("should not be on this side of throw");
    } catch (error_id e) {
      INFO(e);
    } catch (...) {
      FAIL("did not trap error_id");
    }
  }

  SECTION("throw EFOO") {
    try {
      // note we can throw FooErrors::EFOO2
      throw FooErrors::EFOO2;
      FAIL("should not be on this side of throw");
    } catch (error_id e) {
      INFO(e);
    } catch (...) {
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
    CHECK(err_blank.type() == FooErrors::EFOO);
    CHECK(err_blank.type() != FooErrors::EBAR);
    INFO(err_blank.what());
    CHECK(0 == strcmp(err_blank.what(), FooErrors::EFOO));
  }

  SECTION("testing construction with additional string") {
    INFO("testing wrapped values");
    CHECK(err.type() == FooErrors::EFOO);
    CHECK(err.type() != FooErrors::EBAR);
    INFO(err.what());
    CHECK(0 == strcmp(err.what(), msg));
  }
}

TEST_CASE("access wrapper member for error_id", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  INFO("testing comparison wrapped values");
  CHECK(err.type() == FooErrors::EFOO);
  CHECK(err.type() != FooErrors::EBAR);
}

TEST_CASE("access wrapper operator for error overload", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  CHECK(err == FooErrors::EFOO);
  CHECK(err != FooErrors::EBAR);
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
      CHECK(N::new_bar != FooErrors::EBAR);
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
