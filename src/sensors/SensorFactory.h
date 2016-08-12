/*
 * SensorFactory.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SENSORFACTORY_H_
#define SENSORFACTORY_H_

#include "Singleton.h"

#include "Compass.h"
#include "RangeFinder.h"
#include "Accelerometer.h"
#include "Gyroscope.h"

typedef enum
{
	COMPASS_HMC5883L,
	COMPASS_LSM9DS1
} eCompassType;

typedef enum
{
	RANGEFINDER_2Y0A21
} eRangeFinderType;

typedef enum
{
	ACCEL_LSM9DS1
} eAccelType;

typedef enum
{
	GYRO_LSM9DS1
} eGyroType;

class CSensorFactory : public CSingleton<CSensorFactory>
{
	friend class CSingleton<CSensorFactory>;
protected:
	CSensorFactory();
	~CSensorFactory();
public:
	CCompass * createCompass (eCompassType type);
	CCompass * getCompass (void);
	CRangeFinder * createRangeFinder (eRangeFinderType type);
	CRangeFinder * getRangeFinder (void);
	CAccelerometer* createAccelerometer (eAccelType type);
	CAccelerometer * getAccelerometer (void);
	CGyroscope* createGyroscope (eGyroType type);
	CGyroscope * getGyroscope (void);

private:
	CCompass * m_Compass;
	CRangeFinder * m_RangeFinder;
	CAccelerometer * m_Accel;
	CGyroscope * m_Gyro;

};

#endif /* SENSORFACTORY_H_ */
