/*
 * Compass.cpp
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#include "Compass.h"

#include <math.h>

#define HMC5883L_I2C_ADD 0x1E

#define CONF_REG_A 0x00
#define CONF_REG_B 0x01
#define MODE_REG 0x02
#define DATA_REG 0x03

#define MODE_MES_CONT 0x00
#define MODE_MES_SINGLE 0x01
#define MODE_MES_IDLE 0x01

#define PI 3.14159265
#define RAD_TO_DEG 57.2957795

#define max(X,Y) (X>Y)?X:Y
#define min(X,Y) (X<Y)?X:Y

float CCompass::ScaleTable [8] = {0.73, 0.92, 1.22, 1.52, 2.27, 2.56, 3.03, 4.35};

CCompass::CCompass() :
m_Scale (4),
m_I2Cbus (NULL),
m_MaxX(-4000),
m_MaxY(-4000),
m_MaxZ(-4000),
m_MinX(4000),
m_MinY(4000),
m_MinZ(4000),
m_AvgX(0),
m_AvgY(0),
m_AvgZ(0)
{

}


void CCompass::setI2Cbus (CI2Cbus * i2c)
{

	m_I2Cbus = i2c;
	writeReg(CONF_REG_A, 0x10); //8 average, 15Hz default, normal measurement.
	writeReg(CONF_REG_B, m_Scale << 5); //Gain = 5.
	writeReg(MODE_REG, MODE_MES_CONT); //8 average, 15Hz default, normal measurement.
}

CCompass::~CCompass() {

}

void CCompass::writeReg (char regadd, char value)
{
	char buff[2];
	buff[0] = regadd,
	buff[1] = value;
	m_I2Cbus->write(HMC5883L_I2C_ADD,buff, 2);
}

float CCompass::getHeading (void)
{
	float heading = 0;
	char address = DATA_REG;
	char buffer [6];
	short sx, sy, sz;
	float fx,fy,fz;
	if (m_I2Cbus == NULL)
	{
		return 0;
	}
	//if (1 == m_I2Cbus->write (HMC5883L_I2C_ADD,&address, 1))
	{

		if (6 == m_I2Cbus->read(HMC5883L_I2C_ADD, buffer,6))
		{
			sx = ((buffer[0])<< 8) + buffer[1] ;
			sy = ((buffer[4])<< 8) + buffer[5];
			sz = (buffer[2]<< 8) + buffer[3];
			fx = -sx * ScaleTable[m_Scale];
			fy = sy * ScaleTable[m_Scale];
			fz = sz * ScaleTable[m_Scale];

			m_MaxX = max (m_MaxX,fx);
			m_MaxY = max (m_MaxY,fy);
			m_MaxZ = max (m_MaxZ,fz);

			m_MinX = min (m_MinX,fx);
			m_MinY = min (m_MinY,fy);
			m_MinZ = min (m_MinZ,fz);

			m_AvgX = ((m_MaxX - m_MinX)/2)-m_MaxX;
			m_AvgY = ((m_MaxY - m_MinY)/2)-m_MaxY;
			m_AvgZ = ((m_MaxZ - m_MinZ)/2)-m_MaxZ;

			fx += m_AvgX;
			fy += m_AvgY;

			heading = atan2(fx,fy);
			if (heading < 0) heading += 2 * PI;
			if (heading > 2 * PI) heading -= 2 * PI;
			heading *= RAD_TO_DEG;

		}

	}
	m_I2Cbus->write (HMC5883L_I2C_ADD,&address, 1);
	//printf ("compass read out: X: %f/%f/%f | : Y: %f/%f/%f,  heading: %f\n",m_MaxX, m_MinX, m_AvgX, m_MaxY, m_MinY, m_AvgY, heading);
	return heading;

}
