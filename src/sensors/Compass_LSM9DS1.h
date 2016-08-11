/*
 * Compass_LSM9DS1.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef COMPASS_LSM9DS1_H_
#define COMPASS_LSM9DS1_H_

#include <Compass.h>

class CCompass_LSM9DS1: public CCompass
{
	friend class CSensorFactory;

protected:
	CCompass_LSM9DS1();
	virtual ~CCompass_LSM9DS1();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual float getHeading (void);
	virtual void stopCalibration ();
	virtual void startCalibration ();


private:

	float m_MaxX;
	float m_MaxY;
	float m_MaxZ;
	float m_MinX;
	float m_MinY;
	float m_MinZ;
	short m_CalX;
	short m_CalY;
	short m_CalZ;


};

#endif /* COMPASS_LSM9DS1_H_ */
