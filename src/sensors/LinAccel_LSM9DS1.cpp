/*
 * LinAccelLSM9DS1.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <LinAccel_LSM9DS1.h>
#include <cstdio>

#define LSM9DS1_GXL_I2C_ADD 	0x6A

//#define ACT_THS					0x04
//#define ACT_DUR					0x05

#define INT_GEN_CFG_XL 			0x06
#define INT_GEN_THS_X_XL 		0x07
#define INT_GEN_THS_Y_XL 		0x08
#define INT_GEN_THS_Z_XL 		0x09
#define INT_GEN_DUR_XL 			0x0A
#define INT1_CTRL				0x0C
#define INT2_CTRL				0x0D

#define WHO_AM_I				0x0F
#define WHO_AM_I_VAL			0x68

#define OUT_TEMP_L				0x15
#define OUT_TEMP_H				0x16

#define CTRL_REG4				0x1E
#define CTRL_REG5_XL			0x1F
#define CTRL_REG6_XL			0x20
#define CTRL_REG7_XL			0x21
#define CTRL_REG8				0x22
#define CTRL_REG9				0x23
#define CTRL_REG10				0x24

#define INT_GEN_SRC_XL			0x26
#define STATUS_REG				0x27

#define	OUT_X_L_XL				0x28
#define	OUT_X_H_XL				0x29
#define	OUT_Y_L_XL				0x2A
#define	OUT_Y_H_XL				0x2B
#define	OUT_Z_L_XL				0x2C
#define	OUT_Z_H_XL				0x2D

#define FIFO_CTRL				0x2E
#define FIFO_SRC				0x2F

#define CTRL_REG5_XL_VAL		0x38	//No decimation, x,y and z enabled.
#define CTRL_REG6_XL_VAL		0x20	//data rate 10Hz, full scale 2g, auto bandwidth
#define CTRL_REG7_XL_VAL		0x00	//High res disabled, filtering disabled.
#define CTRL_REG8_XL_VAL		0x04	//auto increment address active.

CLinAccel_LSM9DS1::CLinAccel_LSM9DS1() {


}

CLinAccel_LSM9DS1::~CLinAccel_LSM9DS1() {

}

void CLinAccel_LSM9DS1::setBus (CI2Cbus * bus)
{
	unsigned char buff[5];
	unsigned char whoAmI = 0;

	m_I2Cbus = bus;

	if (m_I2Cbus != NULL)
	{
		m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, WHO_AM_I, &whoAmI, 1);

		if (whoAmI != WHO_AM_I_VAL)
		{
			fprintf (stderr, "[LSM9DS1 XL] Error wrong device detected : %d",whoAmI);
			return;
		}

		buff[0] = CTRL_REG5_XL,
		buff[1] = CTRL_REG5_XL_VAL;
		buff[2] = CTRL_REG6_XL_VAL;
		buff[3] = CTRL_REG7_XL_VAL;
		buff[4] = CTRL_REG8_XL_VAL;


		m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 5);
	}
}

sLinAccel CLinAccel_LSM9DS1::getAccel (void)
{
	unsigned char buffer[6];
	sLinAccel ret = {0,0,0};
	short sx,sy,sz;
	if (m_I2Cbus != NULL)
	{
		if (6 == m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_X_L_XL, buffer,6))
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
