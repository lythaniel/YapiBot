/*
 * Gyroscope.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef GYROSCOPE_H_
#define GYROSCOPE_H_

#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} sAngularRate;

class CGyroscope {
		friend class CSensorFactory;
protected:
		CGyroscope() : m_I2Cbus(NULL) {};
	virtual ~CGyroscope() {};

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual sAngularRate getAngularRate (void) = 0;

protected:
	CI2Cbus * m_I2Cbus;
};



#endif /* GYROSCOPE_H_ */
