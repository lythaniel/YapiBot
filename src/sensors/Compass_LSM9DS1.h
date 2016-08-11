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
	virtual float32_t getHeading (void);
	virtual void stopCalibration ();
	virtual void startCalibration ();


private:

	float32_t m_MaxX;
	float32_t m_MaxY;
	float32_t m_MaxZ;
	float32_t m_MinX;
	float32_t m_MinY;
	float32_t m_MinZ;
	int16_t m_CalX;
	int16_t m_CalY;
	int16_t m_CalZ;


};

#endif /* COMPASS_LSM9DS1_H_ */
