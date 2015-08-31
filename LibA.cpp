/*
 * LibA.cpp
 *
 *  Created on: 31 Aug 2015
 *      Author: patrick
 */

#include "LibA.h"

#include "errorcode.hpp"

const char LibA::EFOO[] = SCOPE_ERROR("GRP", "FOO", "Foo clobbered BAR on use");
const char LibA::EBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char LibA::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");




error_code LibA::return_me(int input)
{
	switch (input)
	{
		case 0:
			return LibA::EFOO;

		case 1:
			return LibA::EBAR;

		default:
			return LibA::EPOR;
	}
}


typed_error<LibA::EFOO> LibA::get_foo(const char * message)
{
	return typed_error<LibA::EFOO>(message);
}


void LibA::foo_me(const char * message)
{
	throw typed_error<LibA::EFOO>(message);
}

void LibA::bar_me(const char * message)
{
	throw typed_error<LibA::EBAR>(message);
}

void LibA::suprise_me(const char * message)
{
	throw typed_error<LibA::EPOR>(message);
}




