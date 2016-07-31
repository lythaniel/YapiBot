/*
 * RangeFinder.h
 *
 *  Created on: 1 janv. 2015
 *      Author: lythaniel
 */

#ifndef RANGEFINDER_H_
#define RANGEFINDER_H_

#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

class CRangeFinder
{
	friend class CSensorFactory;

protected:
	CRangeFinder() : m_I2Cbus(NULL) {}
	virtual ~CRangeFinder(){}

public:
	virtual void setBus (CI2Cbus * bus) {m_I2Cbus = bus;}
	virtual int getRange (void) = 0;

protected:
	CI2Cbus * m_I2Cbus;

};

#endif /* RANGEFINDER_H_ */
