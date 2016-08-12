/*
 * AccelGyro_LSM9DS1.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef ACCELGYRO_LSM9DS1_H_
#define ACCELGYRO_LSM9DS1_H_

#include <Accelerometer.h>
#include <Gyroscope.h>

class CAccelGyro_LSM9DS1: public CAccelerometer, public CGyroscope
{
	friend class CSensorFactory;

protected:
	CAccelGyro_LSM9DS1();
	~CAccelGyro_LSM9DS1();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual sAccel getAccel (void);
	virtual sAngularRate getAngularRate (void);


};

#endif /* ACCELGYRO_LSM9DS1_H_ */
