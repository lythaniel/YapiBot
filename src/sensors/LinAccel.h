/*
 * LinAccel.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef ACCEL_H_
#define ACCEL_H_


#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} sLinAccel;

class CLinAccel {
		friend class CSensorFactory;
protected:
		CLinAccel() : m_I2Cbus(NULL) {};
	virtual ~CLinAccel() {};

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual sLinAccel getAccel (void) = 0;

protected:
	CI2Cbus * m_I2Cbus;
};



#endif /* ACCEL_H_ */
