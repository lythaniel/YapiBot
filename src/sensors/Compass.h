/*
 * Compass.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef COMPASS_H_
#define COMPASS_H_

#include <stddef.h>
#include "I2Cbus.h"

typedef struct {
	float32_t x;
	float32_t y;
	float32_t z;
} sMagField;

class CSensorFactory;

class CCompass {
		friend class CSensorFactory;
protected:
	CCompass() : m_I2Cbus(NULL), m_Calib(false) {};
	virtual ~CCompass() {};

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual float32_t getHeading (void) = 0;
	virtual void startCompassCalibration () {m_Calib = true;}
	virtual void stopCompassCalibration (sMagField offset, sMagField scale) {m_Calib = false;}
	virtual sMagField getMagField (void) = 0;
	virtual bool magFieldAvailable (void) = 0;

protected:
	CI2Cbus * m_I2Cbus;
	bool m_Calib;
};

#endif /* COMPASS_H_ */
