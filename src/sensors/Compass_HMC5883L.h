/*
 * Compass_HMC5883L.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef COMPASS_HMC5883L_H_
#define COMPASS_HMC5883L_H_


#include "Compass.h"


class CCompass_HMC5883L : public CCompass
{
	friend class CSensorFactory;

protected:
	CCompass_HMC5883L();
	virtual ~CCompass_HMC5883L();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual float getHeading (void);

private:
	void writeReg (char regadd, char value);

	int m_Scale;

	static float ScaleTable [];
	float m_MaxX;
	float m_MaxY;
	float m_MaxZ;
	float m_MinX;
	float m_MinY;
	float m_MinZ;
	float m_AvgX;
	float m_AvgY;
	float m_AvgZ;

};

#endif /* COMPASS_HMC5883L_H_ */
