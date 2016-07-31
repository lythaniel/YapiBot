/*
 * Compass_LSM9DS1.h
 *
 *  Created on: 31 juil. 2016
 *      Author: lythaniel
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

private:
	//void writeReg (char regadd, char value);

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

#endif /* COMPASS_LSM9DS1_H_ */
