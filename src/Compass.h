/*
 * Compass.h
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#ifndef COMPASS_H_
#define COMPASS_H_

#include "I2Cbus.h"
#include "Singleton.h"

class CCompass : public CSingleton<CCompass>
{
public:
	CCompass();
	~CCompass();

	void setI2Cbus (CI2Cbus * bus);

	float getHeading (void);

private:
	void writeReg (char regadd, char value);

	int m_Scale;
	CI2Cbus * m_I2Cbus;
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

#endif /* COMPASS_H_ */
