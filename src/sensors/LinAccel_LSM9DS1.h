/*
 * LinAccelLSM9DS1.h
 *
 *  Created on: 2 août 2016
 *      Author: lythaniel
 */

#ifndef LINACCELLSM9DS1_H_
#define LINACCELLSM9DS1_H_

#include <LinAccel.h>

class CLinAccel_LSM9DS1: public CLinAccel
{
	friend class CSensorFactory;

protected:
	CLinAccel_LSM9DS1();
	~CLinAccel_LSM9DS1();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual sLinAccel getAccel (void);


};

#endif /* LINACCELLSM9DS1_H_ */
