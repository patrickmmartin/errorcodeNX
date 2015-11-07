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
typedef typed_error<FooErrors::eFOO> foo_err;

typedef typed_error<FooErrors::eBAR> bar_err;

// note that we can deny use of some error_id values, while preserving
// comparison
// here we can't use FooErrors::eFOO2 as it is declared as a pointer, not array
// typedef typed_error<FooErrors::eFOO2> foo2_err;

namespace {
// the concept can be used directly for a basic unique error condition
struct N {
  static const char new_bar[];
};

const char N::new_bar[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");

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

  SECTION("throw eFOO") {
    try {
      // note we can't throw FooErrors::eFOO, but they can for us
      throw_eFOO();
      FAIL("should not be on this side of throw");
    } catch (error_id e) {
      INFO(e);
    } catch (...) {
      FAIL("did not trap error_id");
    }
  }

  SECTION("throw eFOO2") {
    try {
      // note we can throw FooErrors::eFOO2
      throw FooErrors::eFOO2;
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
    CHECK((err_blank.type() == FooErrors::eFOO));
    CHECK((err_blank.type() != FooErrors::eBAR));
    INFO(err_blank.what());
    CHECK((0 == strcmp(err_blank.what(), FooErrors::eFOO)));
  }

  SECTION("testing construction with additional string") {
    INFO("testing wrapped values");
    CHECK((err.type() == FooErrors::eFOO));
    CHECK((err.type() != FooErrors::eBAR));
    INFO(err.what());
    CHECK(!strcmp(err.what(), msg));
  }
}

TEST_CASE("access wrapper member for error_id", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  INFO("testing comparison wrapped values");
  CHECK((err.type() == FooErrors::eFOO));
  CHECK((err.type() != FooErrors::eBAR));
}

TEST_CASE("access wrapper operator for error overload", "[exceptions]") {

  // or the template used and value can be inspected, along with the additional
  // payload
  foo_err err("foo is not a bar - thirst ensues");

  CHECK((err == FooErrors::eFOO));
  CHECK((err != FooErrors::eBAR));
}

TEST_CASE("ensure throwing exception instances works", "[exceptions]") {

  // another possible use of the template class is to have highly specific
  // exception handling

  SECTION("throw and catch typed_error<FooErrors::eFOO>") {
    try {
      throw typed_error_lite<FooErrors::eFOO>();
    } catch (typed_error<FooErrors::eFOO> &e) {
      FAIL("caught in foo_err handler");
    } catch (typed_error_lite<FooErrors::eFOO> &e) {
      INFO("caught typed_error_lite<FooErrors::eFOO>");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("throw and catch typed_error<FooErrors::eFOO>") {
    try {
      throw foo_err("foo != bar");
    } catch (typed_error<FooErrors::eFOO> &e) {
      INFO("caught in foo_err handler");
      CHECK(!strcmp(e.what(), "foo != bar"));
    } catch (bar_err &e) {
      FAIL("Fell through to bar_err handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }

  SECTION("throw and catch typed_error<FooErrors::eBAR>") {
    try {
      throw bar_err("bazong not convertible to bar");
    } catch (typed_error<FooErrors::eFOO> &e) {
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
      CHECK((N::new_bar != FooErrors::eBAR));
      throw typed_error<N::new_bar>("bazong not convertible to bar");
    } catch (typed_error<N::new_bar> &e) {
      INFO("in std::exception handler");
    } catch (typed_error<FooErrors::eBAR> &e) {
      FAIL("in typed_error<FooErrors::eBAR> handler");
    } catch (...) {
      FAIL("Fell through to catch all handler");
    }
  }
}

TEST_CASE("ensure handling thrown exception instances works", "[exceptions]") {

  // check specific exception handling
	try {
	  LibA::foo_me("FOO");
	} catch (typed_error<LibA::eFOO> &e) {
	  INFO("caught in LibA::eFOO handler");
	} catch (...) {
	  FAIL("Fell through to catch all handler for LibA::foo_me()");
	}

	try {
	  LibA::bar_me("BAR");
	} catch (typed_error<LibA::eBAR> &e) {
	  INFO("Caught in LibA::eFOO handler");
	} catch (...) {
		  FAIL("Fell through to catch all handler for LibA::bar_me()");
	}

	try {
	  LibA::suprise_me("SURPRISE");
	} catch (typed_error<LibA::eBAR> &e) {
	  FAIL("Caught in LibA::eBAR handler");
	} catch (std::exception &e) {
	  INFO(e.what());
	} catch (...) {
		  FAIL("Fell through to catch all handler for LibA::surprise_me()");
	}
}
