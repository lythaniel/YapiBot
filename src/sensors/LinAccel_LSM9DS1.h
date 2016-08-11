/*
 * LinAccelLSM9DS1.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
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
