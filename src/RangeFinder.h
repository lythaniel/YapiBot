/*
 * RangeFinder.h
 *
 *  Created on: 1 janv. 2015
 *      Author: lythaniel
 */

#ifndef RANGEFINDER_H_
#define RANGEFINDER_H_

#include "Singleton.h"
#include "I2Cbus.h"

class CRangeFinder: public CSingleton<CRangeFinder> {
public:
	CRangeFinder();
	~CRangeFinder();

	void setI2Cbus (CI2Cbus * bus);
	int getRange (void);

private:
	CI2Cbus * m_I2Cbus;

};

#endif /* RANGEFINDER_H_ */
