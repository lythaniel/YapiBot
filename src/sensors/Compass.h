/*
 * Compass.h
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#ifndef COMPASS_H_
#define COMPASS_H_

#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

class CCompass {
		friend class CSensorFactory;
protected:
	CCompass() : m_I2Cbus(NULL) {};
	virtual ~CCompass() {};

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual float getHeading (void) = 0;

protected:
	CI2Cbus * m_I2Cbus;
};

#endif /* COMPASS_H_ */
