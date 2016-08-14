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
	virtual float32_t getHeading (void);
	virtual sMagField getMagField (void);
	virtual bool magFieldAvailable (void);


private:
	void writeReg (int8_t regadd, int8_t value);

	int32_t m_Scale;

	static float32_t ScaleTable [];
	float32_t m_MaxX;
	float32_t m_MaxY;
	float32_t m_MaxZ;
	float32_t m_MinX;
	float32_t m_MinY;
	float32_t m_MinZ;
	float32_t m_AvgX;
	float32_t m_AvgY;
	float32_t m_AvgZ;

};

#endif /* COMPASS_HMC5883L_H_ */
