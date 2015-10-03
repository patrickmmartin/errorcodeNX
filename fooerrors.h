/*
 * fooerrors.h
 *
 *  Created on: 3 Oct 2015
 *      Author: patrick
 */

#ifndef FOOERRORS_H_
#define FOOERRORS_H_

class FooErrors {
public:
  static const char EFOO[];
  static const char EBAR[];
  static const char EPOR[];
  static const char *EFOO2;
};

void throw_EFOO();

#endif /* FOOERRORS_H_ */
