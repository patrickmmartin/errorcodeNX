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
  static const char eFOO[];
  static const char eBAR[];
  static const char ePOR[];
  static const char *eFOO2;
};

void throw_eFOO();

#endif /* FOOERRORS_H_ */
