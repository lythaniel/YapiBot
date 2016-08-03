/*
 * SensorFactory.h
 *
 *  Created on: 29 juil. 2016
 *      Author: lythaniel
 */

#ifndef SENSORFACTORY_H_
#define SENSORFACTORY_H_

#include "Singleton.h"

#include "Compass.h"
#include "RangeFinder.h"
#include "LinAccel.h"

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
	LINACCEL_LSM9DS1
} eLinAccelType;

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
	CLinAccel* createLinAccel (eLinAccelType type);
	CLinAccel * getLinAccel (void);

private:
	CCompass * m_Compass;
	CRangeFinder * m_RangeFinder;
	CLinAccel * m_LinAccel;

};

#endif /* SENSORFACTORY_H_ */
