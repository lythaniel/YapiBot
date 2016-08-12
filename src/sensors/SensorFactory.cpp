/*
 * SensorFactory.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <SensorFactory.h>
#include "Compass_HMC5883L.h"
#include "Compass_LSM9DS1.h"
#include "RangeFinder_2Y0A21.h"
#include "AccelGyro_LSM9DS1.h"


CSensorFactory::CSensorFactory() :
m_Compass(NULL),
m_RangeFinder(NULL),
m_Accel(NULL),
m_Gyro(NULL)
{


}

CSensorFactory::~CSensorFactory() {
	if (m_Compass != NULL)
	{
		delete m_Compass;
	}
	if (m_RangeFinder != NULL)
	{
		delete m_RangeFinder;
	}
	if (m_Accel != NULL)
	{
		delete m_Accel;
	}
}


CCompass * CSensorFactory::createCompass (eCompassType type)
{
	if (m_Compass == NULL)
	{
		switch (type)
		{
		case COMPASS_HMC5883L:
			m_Compass = new CCompass_HMC5883L();
			break;
		case COMPASS_LSM9DS1:
			m_Compass = new CCompass_LSM9DS1();
			break;
		default:
			fprintf (stderr, "[SENSORS] Impossible to create unknown compass type");
			break;
		}
	}
	else
	{
		fprintf (stdout, "[SENSORS] Warning, trying to create compass while it already exists.");
	}

	return m_Compass;

}

CCompass * CSensorFactory::getCompass (void)
{
	return m_Compass;
}

CRangeFinder * CSensorFactory::createRangeFinder (eRangeFinderType type)
{
	if (m_RangeFinder == NULL)
	{
		switch (type)
		{
		case RANGEFINDER_2Y0A21:
			m_RangeFinder = new CRangeFinder_2Y0A21();
			break;
		default:
			fprintf (stderr, "[SENSORS] Impossible to create unknown range finder type");
			break;
		}
	}
	else
	{
		fprintf (stdout, "[SENSORS] Warning, trying to create range finder while it already exists.");
	}

	return m_RangeFinder;

}

CRangeFinder * CSensorFactory::getRangeFinder (void)
{
	return m_RangeFinder;
}

CAccelerometer * CSensorFactory::createAccelerometer (eAccelType type)
{
	if (m_Accel == NULL)
	{
		switch (type)
		{
		case ACCEL_LSM9DS1:
			if (m_Gyro == NULL)
			{
				m_Accel = new CAccelGyro_LSM9DS1();
			}
			else
			{
				m_Accel = dynamic_cast<CAccelerometer *>(m_Accel);
			}
			break;
		default:
			fprintf (stderr, "[SENSORS] Impossible to create unknown linear accelerometer type");
			break;
		}
	}
	else
	{
		fprintf (stdout, "[SENSORS] Warning, trying to create linear accelerometer while it already exists.");
	}

	return m_Accel;

}

CAccelerometer * CSensorFactory::getAccelerometer (void)
{
	return m_Accel;
}


CGyroscope * CSensorFactory::createGyroscope (eGyroType type)
{
	if (m_Gyro == NULL)
	{
		switch (type)
		{
		case GYRO_LSM9DS1:
			if (m_Accel == NULL)
			{
				m_Gyro = new CAccelGyro_LSM9DS1();
			}
			else
			{
				m_Gyro = dynamic_cast<CGyroscope * >(m_Accel);
			}
			break;
		default:
			fprintf (stderr, "[SENSORS] Impossible to create unknown gyroscope type");
			break;
		}
	}
	else
	{
		fprintf (stdout, "[SENSORS] Warning, trying to create gyroscope while it already exists.");
	}

	return m_Gyro;

}

CGyroscope * CSensorFactory::getGyroscope (void)
{
	return m_Gyro;
}
