/*
 * errorcode.hpp
 *
 *  Created on: 26 Aug 2015
 *      Author: Patrick
 */

#ifndef ERRORCODE_HPP_
#define ERRORCODE_HPP_


/*
 * main.cpp
 *
 *  Created on: 26 Aug 2015
 *  Author: Patrick Martin
 */

#include <iostream>

// ok, here's the big reveal...
typedef char const * error_code;

/* Helper for concatenating argument in a standard
 * This is just a macro, and as such the semantics of the argument list
 * need to be maintained.
 * If different usage is required, define a new macro, or overwrite the
 * definition
 */
#define SCOPE_ERROR(grp, pkg, error_str) grp"-"pkg": "error_str


// we can define a simple template parameterised upon "value"
// this one has a base type and additional info
template<error_code errtype>
class typed_error : public std::exception
{
public:
	typed_error(const char * what = "") :
			_what(what) {};
	virtual const char* what() const throw()
	{
		// TODO(PMM) - most things are allocating no memory - reducing churn would be desirable?
		return (std::string(errtype) + ". " + _what).c_str();
	}
	const char * type() const
	{
		return errtype;
	}
	operator const char *()
	{
		return errtype;
	}

private:
	const char* _what;
};


#endif /* ERRORCODE_HPP_ */
