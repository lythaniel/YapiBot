/*
 * Compass_LSM9DS1.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <Compass_LSM9DS1.h>
#include <cstdio>
#include <math.h>
#include "Settings.h"
#include <unistd.h>
#include <string.h>

#define LSM9DS1_MAG_I2C_ADD 0x1C

#define OFFSET_X_REG_L_M 	0x05
#define OFFSET_X_REG_H_M 	0x06
#define OFFSET_Y_REG_L_M 	0x07
#define OFFSET_Y_REG_H_M 	0x08
#define OFFSET_Z_REG_L_M 	0x09
#define OFFSET_Z_REG_H_M 	0x0A

#define WHO_AM_I 		 	0x0F
#define WHO_AM_I_VAL		0x3D

#define CTRL_REG1_M		 	0x20
#define CTRL_REG2_M		 	0x21
#define CTRL_REG3_M		 	0x22
#define CTRL_REG4_M		 	0x23
#define CTRL_REG5_M		 	0x24

#define STATUS_REG_M	 	0x27

#define OUT_X_L_M 		 	0x28
#define OUT_X_H_M 		 	0x29
#define OUT_Y_L_M 		 	0x2A
#define OUT_Y_H_M 		 	0x2B
#define OUT_Z_L_M 		 	0x2C
#define OUT_Z_H_M 		 	0x2D

#define INT_CFG_M		 	0x30
#define INT_SRC_M		 	0x31
#define INT_THS_L			0x32
#define INT_THS_H			0x33


#define PI 3.14159265
#define RAD_TO_DEG 57.2957795

#define max(X,Y) (X>Y)?X:Y
#define min(X,Y) (X<Y)?X:Y

//LSM9DS1 MAG CONFIGURATION
#define CTRL_REG1_M_VAL		 	0xEC //TEMP_COMP on, x/y ultra high perf, rate = 5Hz, FAST_ODR off, Self test off
#define CTRL_REG2_M_VAL		 	0x20 //Full scale = 8 gauss, Reboot off, Soft reset off
#define CTRL_REG3_M_VAL		 	0x00 //I2C enable, Low power off, SPI enable, continuous mode
#define CTRL_REG4_M_VAL		 	0xC0 //z ultra high perf, lsb at lower address.
#define CTRL_REG5_M_VAL		 	0x00 //Fast data off, block data update off.




CCompass_LSM9DS1::CCompass_LSM9DS1() :
m_MaxX(-32769),
m_MaxY(-32769),
m_MaxZ(-32769),
m_MinX(32769),
m_MinY(32769),
m_MinZ(32769)
{
	m_CalX = CSettings::getInstance()->getInt("COMPASS LSM9DS1","CalX",0);
	m_CalY = CSettings::getInstance()->getInt("COMPASS LSM9DS1","CalY",0);
	m_CalZ = CSettings::getInstance()->getInt("COMPASS LSM9DS1","CalZ",0);
}

CCompass_LSM9DS1::~CCompass_LSM9DS1() {
	// TODO Set compass to low power and reset ? Need to be sure that the bus still exist.
}

void CCompass_LSM9DS1::setBus (CI2Cbus * bus)
{
	uint8_t buff[7];
	uint8_t whoAmI = 0;

	m_I2Cbus = bus;

	if (m_I2Cbus != NULL)
	{

		m_I2Cbus->read(LSM9DS1_MAG_I2C_ADD, WHO_AM_I, &whoAmI, 1);

		if (whoAmI != WHO_AM_I_VAL)
		{
			fprintf (stderr, "[LSM9DS1 MAG] Error wrong device detected : %d",whoAmI);
			return;
		}

		buff[0] = CTRL_REG1_M,
		buff[1] = CTRL_REG1_M_VAL;
		buff[2] = CTRL_REG2_M_VAL;
		buff[3] = CTRL_REG3_M_VAL;
		buff[4] = CTRL_REG4_M_VAL;
		buff[5] = CTRL_REG5_M_VAL;

		m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 6);

		buff[0] = OFFSET_X_REG_L_M,
		buff[1] = (m_CalX) & 0xFF;
		buff[2] = (m_CalX >> 8) & 0xFF;
		buff[3] = (m_CalY) & 0xFF;
		buff[4] = (m_CalY >> 8) & 0xFF;
		buff[5] = (m_CalZ) & 0xFF;
		buff[6] = (m_CalZ >> 8) & 0xFF;

		m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 7);

	}

}
void CCompass_LSM9DS1::startCalibration ()
{
	uint8_t buff[7];

	//Clear offset registers during calibration.
	memset (buff, 0x00, 7); //Clear buffer.
	buff[0] = OFFSET_X_REG_L_M;

	if (m_I2Cbus != NULL)
	{
		m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 7);
	}

	usleep (200000); //200ms Allow some time for value to be written.

	//Init, calibration value.
	m_MaxX = -32769;
	m_MaxY = -32769;
	m_MaxZ = -32769;
	m_MinX = 32769;
	m_MinY = 32769;
	m_MinZ = 32769;

	m_Calib = true;

}

void CCompass_LSM9DS1::stopCalibration ()
{
	m_Calib = false;
	uint8_t buff[7];

	float32_t avgX, avgY, avgZ;

	//Calculate new median value for each axis.
	avgX = ((m_MaxX + m_MinX)/2);
	avgY = ((m_MaxY + m_MinY)/2);
	avgZ = ((m_MaxZ + m_MinZ)/2);

	//Store to int16_t
	m_CalX = avgX;
	m_CalY = avgY;
	m_CalZ = avgZ;

	//Save new values.
	CSettings::getInstance()->setInt("COMPASS LSM9DS1","CalX",m_CalX);
	CSettings::getInstance()->setInt("COMPASS LSM9DS1","CalY",m_CalY);
	CSettings::getInstance()->setInt("COMPASS LSM9DS1","CalZ",m_CalZ);

	//Load alignment values to chip.
	buff[0] = OFFSET_X_REG_L_M,
	buff[1] = (m_CalX) & 0xFF;
	buff[2] = (m_CalX >> 8) & 0xFF;
	buff[3] = (m_CalY) & 0xFF;
	buff[4] = (m_CalY >> 8) & 0xFF;
	buff[5] = (m_CalZ) & 0xFF;
	buff[6] = (m_CalZ >> 8) & 0xFF;

	m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 7);
}

float32_t CCompass_LSM9DS1::getHeading (void)
{
	float32_t heading = 0;
	uint8_t buffer [6];
	int16_t sx, sy, sz;
	float32_t fx,fy,fz;

	//float32_t norm;
	if (m_I2Cbus == NULL)
	{
		return 0;
	}
	if (6 == m_I2Cbus->read(LSM9DS1_MAG_I2C_ADD, OUT_X_L_M, buffer,6))
	{
		sx = ((buffer[1])<< 8) + buffer[0] ;
		sy = ((buffer[3])<< 8) + buffer[2];
		sz = (buffer[5]<< 8) + buffer[4];
		fx = sx;
		fy = sy;
		fz = sz;


		m_MaxX = max (m_MaxX,fx);
		m_MaxY = max (m_MaxY,fy);
		m_MaxZ = max (m_MaxZ,fz);

		m_MinX = min (m_MinX,fx);
		m_MinY = min (m_MinY,fy);
		m_MinZ = min (m_MinZ,fz);


		//norm = fx * fx + fy * fy + fz * fz;


		heading = atan2(-fx,fy);
		if (heading < 0) heading += 2 * PI;
		if (heading > 2 * PI) heading -= 2 * PI;
		heading *= RAD_TO_DEG;

		//printf ("compass read out: X: %f/%f/%f/%d | Y: %f/%f/%f/%d | Z: %f/%f/%f/%d | heading: %f | norm: %f\n", fx, m_MaxX, m_MinX, m_CalX, fy, m_MaxY, m_MinY, m_CalY, fz, m_MaxZ, m_MinZ, m_CalZ, heading, norm);

	}



	return heading;
}


