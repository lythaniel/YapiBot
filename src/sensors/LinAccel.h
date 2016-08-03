/*
 * Accel.h
 *
 *  Created on: 2 août 2016
 *      Author: lythaniel
 */

#ifndef ACCEL_H_
#define ACCEL_H_


#include <stddef.h>
#include "I2Cbus.h"

class CSensorFactory;

typedef struct {
	int x;
	int y;
	int z;
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
