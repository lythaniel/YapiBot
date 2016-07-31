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

typedef enum
{
	COMPASS_HMC5883L,
	COMPASS_LSM9DS1
} eCompassType;

typedef enum
{
	RANGEFINDER_2Y0A21
} eRangeFinderType;


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

private:
	CCompass * m_Compass;
	CRangeFinder * m_RangeFinder;

};

#endif /* SENSORFACTORY_H_ */
