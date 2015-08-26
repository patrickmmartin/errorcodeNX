/*
 * main.cpp
 *
 *  Created on: 26 Aug 2015
 *  Author: Patrick Martin
 */

#include "errorcode.hpp"

#include <iostream>

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


// TODO(PMM) as an apologia for the lack of switch, example dispatch class to dispatch on error type

template <const char * errtype>
class ErrorDispatcher {
private:
    error_code _errtype;
    void dispatchError() {
        std::cout << "ErrorDispatcher - default handler!" << std::endl;
    };
public:

    ErrorDispatcher() { _errtype = errtype; }

    void operator()()
    {
        // this member is provided by the specialisations
    	dispatchError();
    };

};

template<>
void ErrorDispatcher<FooErrors::EFOO>::dispatchError()
{
    std::cout << "ErrorDispatcher<Foo::EFOO>! NOT default handler - " << _errtype << std::endl;
};



int main(int arc, char * argv[])
{

	// the concept can be used directly for a basic unique error condition
	error_code raw_foo = FooErrors::EBAR;

	if (raw_foo == FooErrors::EFOO)
	{
		std::cout << raw_foo << std::endl;
	}
	else if (raw_foo == FooErrors::EBAR)
	{
		std::cout << raw_foo << std::endl;
	}

	// or the template used and value can be inspected, along with the additional payload
	foo_err err("foo is not a bar - thirst ensues");

	if (err.type() == FooErrors::EFOO)
	{
		std::cout << "FOO ERROR " << err.what() << std::endl;
	}

	if (err.type() == FooErrors::EBAR)
	{
		std::cout << "BAZONG ERROR " << err.what() << std::endl;
	}

	// or make use of the overload
	if (err == FooErrors::EFOO)
	{
		std::cout << "FOO ERROR " << err.what() << std::endl;
	}

	if (err == FooErrors::EBAR)
	{
		std::cout << "BAZONG ERROR " << err.what() << std::endl;
	}


	// another possible use of the template class is to have highly specific exception handling
	try
	{
		throw foo_err("foo != bar");
	}
	catch (foo_err & e)
	{
		std::cout << "FOO THROWN " << e.what() << std::endl;
	}
	catch (bar_err & e)
	{
		std::cout << "BAZONG THROWN " << e.what() << std::endl;
	}

	try
	{
		throw bar_err("bazong not convertible to bar");
	}
	catch (foo_err & e)
	{
		std::cout << "FOO THROWN " << e.what() << std::endl;
	}
	catch (bar_err & e)
	{
		std::cout << "BAZONG THROWN " << e.what() << std::endl;
	}

	// switch statements are basically just a problem -they mandate constants
	// so nothing here

	// TODO(PMM) - this has some use as allowing error handlers to be picked up
	ErrorDispatcher<FooErrors::EFOO>()();

	ErrorDispatcher<FooErrors::EBAR>()();

	// TODO(PMM) - next step is something that will achieve the same goals as a switch [shudder]



}
