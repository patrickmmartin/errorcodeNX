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

// This is a macro and as such the semantics need to be maintained
#define SCOPE_ERROR(grp, pkg, error_str) grp"-"pkg": "error_str



// we can define a simple template parameterised upon "value"
// this one has a base type and additional info
template<error_code errtype>
class error_type : public std::exception
{
public:
	error_type(const char * what = "") :
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
	// TOOD(PMM) is this a help or a hindrance?
	operator const char *()
	{
		return errtype;
	}

private:
	const char* _what;
};


#endif /* ERRORCODE_HPP_ */
