/*
 * main.cpp
 *
 *  Created on: 26 Aug 2015
 *  Author: Patrick Martin
 */

#include "errorcode.hpp"

#include <iostream>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

// this class merely supplies a namespace for holding errors
class FooErrors
{
public:
	static const char EFOO[];
	static const char EBAR[];
};

// this is a hand-crafted error definition
const char FooErrors::EFOO[] = "GRP-FOO: Foo clobbered BAR on use";
const char FooErrors::EBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");

// we can define new unique instances
typedef error_type<FooErrors::EFOO> foo_err;

typedef error_type<FooErrors::EBAR> bar_err;

// or we can use the value directly
error_code const A = FooErrors::EFOO;
error_code const B = FooErrors::EBAR;



TEST_CASE( "test values directly", "[errorcode]" ) {

	// the concept can be used directly for a basic unique error condition
	error_code raw_foo = FooErrors::EBAR;

	INFO("testing raw values");
	CHECK((raw_foo == FooErrors::EBAR));
	CHECK((raw_foo != FooErrors::EFOO));

}

TEST_CASE( "test wrapper for error", "[errorcode]" ) {

	// or the template used and value can be inspected, along with the additional payload
	foo_err err("foo is not a bar - thirst ensues");

	INFO("testing wrapped values");
	CHECK((err.type() == FooErrors::EFOO));
	CHECK((err.type() != FooErrors::EBAR));

}

TEST_CASE( "test wrapper for error overload", "[errorcode]" ) {

	// or the template used and value can be inspected, along with the additional payload
	foo_err err("foo is not a bar - thirst ensues");

	CHECK((err == FooErrors::EFOO));
	CHECK((err != FooErrors::EBAR));

}

TEST_CASE( "test exception instances", "[errorcode]" ) {

	// another possible use of the template class is to have highly specific exception handling
	try
	{
		throw foo_err("foo != bar");
	}
	catch (error_type<FooErrors::EFOO> & e)
	{
		INFO("caught in foo_err handler");
	}
	catch (bar_err & e)
	{
		FAIL("Fell through to bar_err handler");
	}
	catch (...)
	{
		FAIL("Fell through to catch all handler");
	}

	try
	{
		throw bar_err("bazong not convertible to bar");
	}
	catch (error_type<FooErrors::EFOO> & e)
	{
		FAIL("Caught in foo_err handler");
	}
	catch (bar_err & e)
	{
		INFO("in bar_err handler");
	}
	catch (...)
	{
		FAIL("Fell through to catch all handler");
	}
}


// TODO(PMM) as an apologia for the lack of switch, example dispatch class to dispatch on error type

bool dispatch_default_called = false;
bool dispatch_foo_called = false;

template <error_code errtype>
class ErrorDispatcher {
private:
    error_code _errtype;
    void dispatchError() {
        dispatch_default_called = true;
    };
public:

    ErrorDispatcher() { _errtype = errtype; }

    void operator()()
    {
        // this member is provided by the specialisations
    	dispatchError();
    };

};


TEST_CASE( "test static dispatchers", "[errorcode]" ) {

	// switch statements are basically just a problem -they mandate constants
	// so nothing here for a nice clean syntax of switch

	// However we can used template specialisation
	// this caters for having set up a set of handlers in scope prepared in advance
	// there is a single site to inherit all that handling
	// this all works nicely because the code paths are defined at compile time

	dispatch_default_called = false;
	dispatch_foo_called = false;

	ErrorDispatcher<FooErrors::EFOO>()();
	CHECK(dispatch_foo_called);

	ErrorDispatcher<FooErrors::EBAR>()();
	CHECK(dispatch_default_called);

}



template<>
void ErrorDispatcher<FooErrors::EFOO>::dispatchError()
{
    dispatch_foo_called = true;
};



TEST_CASE( "test static dispatchers add case", "[errorcode]" ) {

	// switch statements are basically just a problem -they mandate constants
	// so nothing here for a nice clean syntax of switch

	// However we can use template specialisation, as demonstrated below

	dispatch_default_called = false;
	dispatch_foo_called = false;

	ErrorDispatcher<FooErrors::EFOO>()();
	CHECK(dispatch_foo_called);

	ErrorDispatcher<FooErrors::EBAR>()();
	CHECK(dispatch_default_called);

}


struct None { };

template <typename H, typename T = None>
struct typelist
{
    typedef H Head;
    typedef T Tail;
};


typedef typelist<ErrorDispatcher<FooErrors::EFOO>,
			typelist<ErrorDispatcher<FooErrors::EBAR>, None> >
    	DispatchList;


// TODO(PMM) - next step is something that will achieve the same goals as a switch [shudder]


TEST_CASE( "test dispatcher typelist", "[errorcode]" ) {


	DispatchList displist;

	// TODO(PMM) - next step is something that will test an error_code *value* and match

	error_code foo = FooErrors::EFOO;







}


