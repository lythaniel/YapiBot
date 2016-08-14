/*
 * Compass_HMC5883L.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "Compass_HMC5883L.h"

#include <math.h>

#define HMC5883L_I2C_ADD 0x1E

#define CONF_REG_A 0x00
#define CONF_REG_B 0x01
#define MODE_REG 0x02
#define DATA_REG 0x03
#define STATUS_REG 0x09
#define ID_REG_A	0x10
#define ID_REG_B	0x11
#define ID_REG_C	0x12

#define MODE_MES_CONT 0x00
#define MODE_MES_SINGLE 0x01
#define MODE_MES_IDLE 0x01

#define PI 3.14159265
#define RAD_TO_DEG 57.2957795

#define max(X,Y) (X>Y)?X:Y
#define min(X,Y) (X<Y)?X:Y

float32_t CCompass_HMC5883L::ScaleTable [8] = {0.73, 0.92, 1.22, 1.52, 2.27, 2.56, 3.03, 4.35};

CCompass_HMC5883L::CCompass_HMC5883L() :
m_Scale (4),
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


void CCompass_HMC5883L::setBus (CI2Cbus * i2c)
{

	m_I2Cbus = i2c;
	writeReg(CONF_REG_A, 0x10); //8 average, 15Hz default, normal measurement.
	writeReg(CONF_REG_B, m_Scale << 5); //Gain = 5.
	writeReg(MODE_REG, MODE_MES_CONT); //8 average, 15Hz default, normal measurement.
}

CCompass_HMC5883L::~CCompass_HMC5883L() {

}

void CCompass_HMC5883L::writeReg (int8_t regadd, int8_t value)
{
	uint8_t buff[2];
	buff[0] = regadd,
	buff[1] = value;
	m_I2Cbus->write(HMC5883L_I2C_ADD,buff, 2);
}

float32_t CCompass_HMC5883L::getHeading (void)
{
	float32_t heading = 0;
	uint8_t address = DATA_REG;
	uint8_t buffer [6];
	int16_t sx, sy, sz;
	sMagField field;
	if (m_I2Cbus == NULL)
	{
		return 0;
	}
	if (1 == m_I2Cbus->write (HMC5883L_I2C_ADD,&address, 1))
	{

		if (6 == m_I2Cbus->read(HMC5883L_I2C_ADD, buffer,6))
		{
			sx = ((buffer[0])<< 8) + buffer[1] ;
			sy = ((buffer[4])<< 8) + buffer[5];
			sz = (buffer[2]<< 8) + buffer[3];
			field.x = -sx * ScaleTable[m_Scale];
			field.y = sy * ScaleTable[m_Scale];
			field.z = sz * ScaleTable[m_Scale];

			m_MaxX = max (m_MaxX,field.x);
			m_MaxY = max (m_MaxY,field.y);
			m_MaxZ = max (m_MaxZ,field.z);

			m_MinX = min (m_MinX,field.x);
			m_MinY = min (m_MinY,field.y);
			m_MinZ = min (m_MinZ,field.z);

			m_AvgX = ((m_MaxX - m_MinX)/2)-m_MaxX;
			m_AvgY = ((m_MaxY - m_MinY)/2)-m_MaxY;
			m_AvgZ = ((m_MaxZ - m_MinZ)/2)-m_MaxZ;

			field.x += m_AvgX;
			field.y += m_AvgY;
			field.z += m_AvgZ;

			heading = atan2(field.x,field.y);
			if (heading < 0) heading += 2 * PI;
			if (heading > 2 * PI) heading -= 2 * PI;
			heading *= RAD_TO_DEG;

		}

	}
	//printf ("compass read out: X: %f/%f/%f | : Y: %f/%f/%f,  heading: %f\n",m_MaxX, m_MinX, m_AvgX, m_MaxY, m_MinY, m_AvgY, heading);
	return heading;

}

bool CCompass_HMC5883L::magFieldAvailable (void)
{
	bool ret = false;
	uint8_t address = STATUS_REG;
	uint8_t status;
	if (m_I2Cbus != NULL)
	{
		if (1 == m_I2Cbus->write (HMC5883L_I2C_ADD,&address, 1))
		{

			if (1 == m_I2Cbus->read(HMC5883L_I2C_ADD, &status,1))
			{
				ret = status & 0x1;
			}
		}
	}
	return ret;
}

sMagField CCompass_HMC5883L::getMagField (void)
{
	float32_t heading = 0;
	uint8_t address = DATA_REG;
	uint8_t buffer [6];
	int16_t sx, sy, sz;
	sMagField field = {0,0,0};
	if (m_I2Cbus == NULL)
	{
		return field;
	}
	if (1 == m_I2Cbus->write (HMC5883L_I2C_ADD,&address, 1))
	{

		if (6 == m_I2Cbus->read(HMC5883L_I2C_ADD, buffer,6))
		{
			sx = ((buffer[0])<< 8) + buffer[1] ;
			sy = ((buffer[4])<< 8) + buffer[5];
			sz = (buffer[2]<< 8) + buffer[3];
			field.x = -sx * ScaleTable[m_Scale];
			field.y = sy * ScaleTable[m_Scale];
			field.z = sz * ScaleTable[m_Scale];

			m_MaxX = max (m_MaxX,field.x);
			m_MaxY = max (m_MaxY,field.y);
			m_MaxZ = max (m_MaxZ,field.z);

			m_MinX = min (m_MinX,field.x);
			m_MinY = min (m_MinY,field.y);
			m_MinZ = min (m_MinZ,field.z);

			m_AvgX = ((m_MaxX - m_MinX)/2)-m_MaxX;
			m_AvgY = ((m_MaxY - m_MinY)/2)-m_MaxY;
			m_AvgZ = ((m_MaxZ - m_MinZ)/2)-m_MaxZ;

			field.x += m_AvgX;
			field.y += m_AvgY;
			field.z += m_AvgZ;
		}
	}
	//printf ("compass read out: X: %f/%f/%f | : Y: %f/%f/%f,  heading: %f\n",m_MaxX, m_MinX, m_AvgX, m_MaxY, m_MinY, m_AvgY, heading);
	return field;

}

