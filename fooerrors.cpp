/*
 * fooerrors.cpp
 *
 *  Created on: 3 Oct 2015
 *      Author: patrick
 */

#include "fooerrors.h"
#include "error_id.hpp"

// this is a hand-crafted error definition
const char FooErrors::eFOO[] = "GRP-FOO: Foo clobbered BAR on use";
// and these use the convenience macro
const char FooErrors::eBAR[] = SCOPE_ERROR("GRP", "FOO", "Foo not Bar");
const char FooErrors::ePOR[] = SCOPE_ERROR("GRP", "FOO", "Foo not reparable");

const char *FooErrors::eFOO2 = "GRP-FOO: Foo clobbered BAR on use";

void throw_eFOO() { throw FooErrors::eFOO; }
