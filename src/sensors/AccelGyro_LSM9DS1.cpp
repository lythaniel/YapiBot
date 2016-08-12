/*
 * AccelGyro_LSM9DS1.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <AccelGyro_LSM9DS1.h>
#include <cstdio>

#define LSM9DS1_GXL_I2C_ADD 	0x6A

#define ACT_THS					0x04
#define ACT_DUR					0x05

#define INT_GEN_CFG_XL 			0x06
#define INT_GEN_THS_X_XL 		0x07
#define INT_GEN_THS_Y_XL 		0x08
#define INT_GEN_THS_Z_XL 		0x09
#define INT_GEN_DUR_XL 			0x0A

#define REFERENCE_G				0x0B

#define INT1_CTRL				0x0C
#define INT2_CTRL				0x0D

#define WHO_AM_I				0x0F
#define WHO_AM_I_VAL			0x68

#define CTRL_REG1_G				0x10
#define CTRL_REG2_G				0x11
#define CTRL_REG3_G				0x12

#define ORIENT_CFG_G			0x13
#define CINT_GEN_SRC			0x14

#define OUT_TEMP_L				0x15
#define OUT_TEMP_H				0x16

#define STATUS_REG_G			0x17

#define OUT_X_L_G				0x18
#define OUT_X_H_G				0x19
#define OUT_Y_L_G				0x1A
#define OUT_Y_H_G				0x1B
#define OUT_Z_L_G				0x1C
#define OUT_Z_H_G				0x1D


#define CTRL_REG4				0x1E
#define CTRL_REG5_XL			0x1F
#define CTRL_REG6_XL			0x20
#define CTRL_REG7_XL			0x21
#define CTRL_REG8				0x22
#define CTRL_REG9				0x23
#define CTRL_REG10				0x24

#define INT_GEN_SRC_XL			0x26
#define STATUS_REG_XL			0x27

#define	OUT_X_L_XL				0x28
#define	OUT_X_H_XL				0x29
#define	OUT_Y_L_XL				0x2A
#define	OUT_Y_H_XL				0x2B
#define	OUT_Z_L_XL				0x2C
#define	OUT_Z_H_XL				0x2D

#define FIFO_CTRL				0x2E
#define FIFO_SRC				0x2F

#define INT_GEN_CFG_G			0x30
#define INT_GEN_THS_XH_G		0x31
#define INT_GEN_THS_XL_G		0x32
#define INT_GEN_THS_YH_G		0x33
#define INT_GEN_THS_YL_G		0x34
#define INT_GEN_THS_ZH_G		0x35
#define INT_GEN_THS_ZL_G		0x36
#define INT_GEN_DUR_G			0x37


#define CTRL_REG1_G_VAL			0xC0	//Data rate = 952Hz / FS = 245dps / BW= 00 (33Hz with this data rate)
#define CTRL_REG2_G_VAL			0x00	//Filtering disabled.
#define CTRL_REG3_G_VAL			0x00	//Low power disabled, HPF disabled, HP cutoff 0000
#define ORIENT_CFG_G_VAL		0x00	//Default orientation.
#define CTRL_REG4_VAL			0x38	//gyro X,Y,Z enabled, latched interrupt disabled, 4D position disabled.

#define CTRL_REG5_XL_VAL		0x38	//No decimation, x,y and z enabled.
#define CTRL_REG6_XL_VAL		0x20	//data rate 10Hz, full scale 2g, auto bandwidth
#define CTRL_REG7_XL_VAL		0x00	//High res disabled, filtering disabled.
#define CTRL_REG8_XL_VAL		0x04	//auto increment address active.

CAccelGyro_LSM9DS1::CAccelGyro_LSM9DS1() {


}

CAccelGyro_LSM9DS1::~CAccelGyro_LSM9DS1() {

}

void CAccelGyro_LSM9DS1::setBus (CI2Cbus * bus)
{
	uint8_t buff[5];
	uint8_t whoAmI = 0;

	CAccelerometer::m_I2Cbus = bus;

	if (CAccelerometer::m_I2Cbus != NULL)
	{
		//Check is this is the correct device.
		CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, WHO_AM_I, &whoAmI, 1);

		if (whoAmI != WHO_AM_I_VAL)
		{
			fprintf (stderr, "[LSM9DS1 XL] Error wrong device detected : %d",whoAmI);
			return;
		}

		//Configure the Accelerometer.
		buff[0] = CTRL_REG5_XL;
		buff[1] = CTRL_REG5_XL_VAL;
		buff[2] = CTRL_REG6_XL_VAL;
		buff[3] = CTRL_REG7_XL_VAL;
		buff[4] = CTRL_REG8_XL_VAL;
		CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 5);

		//Configure the gyroscope.
		buff[0] = CTRL_REG1_G;
		buff[1] = CTRL_REG1_G_VAL;
		buff[2] = CTRL_REG2_G_VAL;
		buff[3] = CTRL_REG3_G_VAL;
		buff[4] = ORIENT_CFG_G_VAL;
		CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 5);

		buff[0] = CTRL_REG4;
		buff[1] = CTRL_REG4_VAL;
		CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 2);


	}
}

sAccel CAccelGyro_LSM9DS1::getAccel (void)
{
	uint8_t buffer[6];
	sAccel ret = {0,0,0};
	int16_t sx,sy,sz;
	if (CAccelerometer::m_I2Cbus != NULL)
	{
		if (6 == CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_X_L_XL, buffer,6))
		{
			sx = ((buffer[1])<< 8) + buffer[0] ;
			sy = ((buffer[3])<< 8) + buffer[2];
			sz = (buffer[5]<< 8) + buffer[4];
			ret.x = -sy;
			ret.y = sx;
			ret.z = sz;
		}
		//printf ("[LSM9DS1 XL] Acceleration: x= %d | y = %d | z = %d\n", ret.x, ret.y, ret.z);
	}
	return ret;

}

sAngularRate CAccelGyro_LSM9DS1::getAngularRate (void)
{
	uint8_t buffer[6];
	sAngularRate ret = {0,0,0};
	int16_t sx,sy,sz;
	if (CAccelerometer::m_I2Cbus != NULL)
	{
		if (6 == CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_X_L_G, buffer,6))
		{
			sx = ((buffer[1])<< 8) + buffer[0] ;
			sy = ((buffer[3])<< 8) + buffer[2];
			sz = (buffer[5]<< 8) + buffer[4];
			ret.x = -sy;
			ret.y = sx;
			ret.z = sz;
		}
		//printf ("[LSM9DS1 GYRO] Angular rate: x= %d | y = %d | z = %d\n", ret.x, ret.y, ret.z);
	}
	return ret;

}
