/*
 * SensorFactory.cpp
 *
 *  Created on: 29 juil. 2016
 *      Author: lythaniel
 */

#include <SensorFactory.h>
#include "Compass_HMC5883L.h"
#include "Compass_LSM9DS1.h"
#include "RangeFinder_2Y0A21.h"

CSensorFactory::CSensorFactory() :
m_Compass(NULL),
m_RangeFinder(NULL)
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
