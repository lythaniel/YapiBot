/*
 * Accelerometer.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_


#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

typedef struct {
	float32_t x;
	float32_t y;
	float32_t z;
} sAccel;

class CAccelerometer {
		friend class CSensorFactory;
protected:
	CAccelerometer() : m_I2Cbus(NULL) {};
	virtual ~CAccelerometer() {};

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual sAccel getAccel (void) = 0;
	virtual bool accelSamplesAvailable (void)= 0;

	virtual void startAccelCalibration (void) {}
	virtual void stopAccelCalibration (sAccel offset, sAccel scale) {}



protected:
	CI2Cbus * m_I2Cbus;
};



#endif /* ACCELEROMETER_H_ */
