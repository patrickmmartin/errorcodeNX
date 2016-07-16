/*
 * error_id.hpp
 *
 *  Created on: 26 Aug 2015
 *      Author: Patrick
 */

#ifndef ERROR_ID_HPP_
#define ERROR_ID_HPP_

// error_id defines an array value, which must be initialised from a string literal
// the _content_ of the error_id may or may not be unique, but following on from ODR
// the pointer in a statically linked application must be unique,
// whereas string constants may or not be folded if their content matches

// the rvalue of an error_id is hence
typedef char const error_id[];

// the lvalue for an error_id is hence
typedef char const* error_value;

// although the code will symbolically and semantically work perfectly with string literals that
// [i] collide, [ii] are not human parseable [iii] not strings
// it nevertheless would be a helpful affordance to generate unique readable strings


/* Helper for concatenating argument in a standard
 * This is just a macro, and as such the semantics of the argument list
 * need to be maintained.
 * If different usage is required, define a new macro, or overwrite the
 * definition
 */
#define SCOPE_ERROR(grp, pkg, error_str) grp "-" pkg ": " error_str

// SCOPE_ERROR("GRP", "FOO", "Foo not Bar") == "GRP-FOO: Foo not Bar"

// Organisations can exploit other preprocessor
// features to ensure uniqueness of output

#define STRING(X) "" # X
#define TOSTR(X) STRING(X)

#define SCOPE_ERROR_LOCATION(grp, pkg, error_str) \
        __FILE__ ":" TOSTR(__LINE__) " " grp "-" pkg ": " error_str " "

#endif /* ERROR_ID_HPP_ */
