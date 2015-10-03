/*
 * fooerrors.cpp
 *
 *  Created on: 3 Oct 2015
 *      Author: patrick
 */

#include "fooerrors.h"
#include "error_id.hpp"

// this is a hand-crafted error definition
const char FooErrors::EFOO[] = "GRP-FOO: Foo clobbered BAR on use";
// and these use the convenience macro
const char FooErrors::EBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char FooErrors::EPOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

const char *FooErrors::EFOO2 = "GRP-FOO: Foo clobbered BAR on use";

void throw_EFOO() { throw FooErrors::EFOO; }
