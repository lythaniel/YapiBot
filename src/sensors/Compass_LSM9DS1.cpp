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
#include <pigpiod_if2.h>
#include <string.h>

#define LSM9DS1_MAG_I2C_ADD 0x1C

#define LSM9DS1_INT_M_GPIO	6
#define LSM9DS1_DRDY_M_GPIO	13

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
#define CTRL_REG1_M_VAL		 	0xF8 //TEMP_COMP on, x/y ultra high perf, rate = 40Hz, FAST_ODR off, Self test off
#define CTRL_REG2_M_VAL		 	0x20 //Full scale = 8 gauss, Reboot off, Soft reset off
#define CTRL_REG3_M_VAL		 	0x00 //I2C enable, Low power off, SPI enable, continuous mode
#define CTRL_REG4_M_VAL		 	0xC0 //z ultra high perf, lsb at lower address.
#define CTRL_REG5_M_VAL		 	0x00 //Fast data off, block data update off.


static void lsm9ds1_int_m (int32_t pi, uint32_t gpio, uint32_t level, uint32_t tick, void * user)
{
	CCompass_LSM9DS1 * lsm9ds1 = (CCompass_LSM9DS1 *) user;
	lsm9ds1->dataReady ();
}

CCompass_LSM9DS1::CCompass_LSM9DS1() :
m_Pi(-1),
m_IntCallbackId(-1),
m_NumSample(0),
m_BufferInIdx(0),
m_BufferOutIdx(0),
m_InputSampCnt(0)
{
	memset (m_SampleBuffer, 0x00, sizeof(m_SampleBuffer));

	m_CompCalOffset.x = 0.0f;
		m_CompCalOffset.y = 0.0f;
		m_CompCalOffset.z = 0.0f;

		m_CompCalScale.x = 1.0f;
		m_CompCalScale.y = 1.0f;
		m_CompCalScale.z = 1.0f;

/*	m_CompCalOffset.x = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalOffsetX",0.0f);
	m_CompCalOffset.y = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalOffsetY",0.0f);
	m_CompCalOffset.z = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalOffsetZ",0.0f);

	m_CompCalScale.x = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalScaleX",1.0f);
	m_CompCalScale.y = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalScaleY",1.0f);
	m_CompCalScale.z = CSettings::getInstance()->getFloat("COMPASS LSM9DS1","CalScaleZ",1.0f);
*/
}

CCompass_LSM9DS1::~CCompass_LSM9DS1() {
	// TODO Set compass to low power and reset ? Need to be sure that the bus still exist.
	callback_cancel(m_IntCallbackId);
	pigpio_stop(m_Pi);
}
extern int32_t pigpio;
void CCompass_LSM9DS1::setBus (CI2Cbus * bus)
{
	uint8_t buff[7];
	uint8_t whoAmI = 0;

	m_I2Cbus = bus;

	if (m_I2Cbus != NULL)
	{

		//Check that we have the correct device.
		m_I2Cbus->read(LSM9DS1_MAG_I2C_ADD, WHO_AM_I, &whoAmI, 1);

		if (whoAmI != WHO_AM_I_VAL)
		{
			fprintf (stderr, "[LSM9DS1 MAG] Error wrong device detected : %d",whoAmI);
			return;
		}

		//Connect the interrupt handler:
		//Temporary until proper GPIO interface.
		m_Pi = pigpio;//pigpio_start(NULL,NULL); //local gpio
		set_mode (m_Pi, LSM9DS1_DRDY_M_GPIO, PI_INPUT);
		m_IntCallbackId = callback_ex(m_Pi, LSM9DS1_DRDY_M_GPIO, RISING_EDGE, lsm9ds1_int_m, (void *)this);


		//Configure magnetometer.
		buff[0] = CTRL_REG1_M,
		buff[1] = CTRL_REG1_M_VAL;
		buff[2] = CTRL_REG2_M_VAL;
		buff[3] = CTRL_REG3_M_VAL;
		buff[4] = CTRL_REG4_M_VAL;
		buff[5] = CTRL_REG5_M_VAL;
		m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 6);


		int16_t m_CalX = 15641;
		int16_t m_CalY = -750;
		int16_t m_CalZ = 217;

		//Apply calibration
		buff[0] = OFFSET_X_REG_L_M,
		buff[1] = (m_CalX) & 0xFF;
		buff[2] = (m_CalX >> 8) & 0xFF;
		buff[3] = (m_CalY) & 0xFF;
		buff[4] = (m_CalY >> 8) & 0xFF;
		buff[5] = (m_CalZ) & 0xFF;
		buff[6] = (m_CalZ >> 8) & 0xFF;
		m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 7);

		//Dummy reading to clear the DRDY pin.
		m_I2Cbus->read(LSM9DS1_MAG_I2C_ADD, OUT_X_L_M, buff,6);



	}

}
void CCompass_LSM9DS1::startCompassCalibration ()
{
	uint8_t buff[7];

	//Clear offset registers during calibration.
	/*memset (buff, 0x00, 7); //Clear buffer.
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

	m_Calib = true;*/

	m_CompCalOffset.x = 0.0f;
	m_CompCalOffset.y = 0.0f;
	m_CompCalOffset.z = 0.0f;

	m_CompCalScale.x = 1.0f;
	m_CompCalScale.y = 1.0f;
	m_CompCalScale.z = 1.0f;


}

void CCompass_LSM9DS1::stopCompassCalibration (sMagField offset, sMagField scale)
{
	/*m_Calib = false;
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

	m_I2Cbus->write(LSM9DS1_MAG_I2C_ADD,buff, 7);*/
	m_CompCalOffset = offset;
	m_CompCalScale = scale;

	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalOffsetX",m_CompCalOffset.x);
	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalOffsetY",m_CompCalOffset.y);
	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalOffsetZ",m_CompCalOffset.z);

	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalScaleX",m_CompCalScale.x);
	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalScaleY",m_CompCalScale.y);
	CSettings::getInstance()->setFloat("COMPASS LSM9DS1","CalScaleZ",m_CompCalScale.z);

}

float32_t CCompass_LSM9DS1::getHeading (void)
{
	float32_t heading = 0;
	sMagField field = {0, 0, 0};


	if (m_NumSample > 0)
	{

		m_BufferLock.get();
		field = m_SampleBuffer[m_BufferOutIdx++];
		m_BufferOutIdx &= (LDSM9DS1_MAG_CIRC_BUFFER_SIZE-1);
		m_NumSample --;
		if (m_NumSample < 0)
		{
			//Not possible ...
			m_NumSample = 0;
		}
		m_BufferLock.release();
		heading = atan2(-field.x,field.y);
		if (heading < 0) heading += 2 * PI;
		if (heading > 2 * PI) heading -= 2 * PI;
		heading *= RAD_TO_DEG;

		//printf ("compass read out: X: %f/%f/%f/%d | Y: %f/%f/%f/%d | Z: %f/%f/%f/%d | heading: %f\n", field.x, m_MaxX, m_MinX, m_CalX, field.y, m_MaxY, m_MinY, m_CalY, field.z, m_MaxZ, m_MinZ, m_CalZ, heading);

	}

	return heading;
}

sMagField CCompass_LSM9DS1::getMagField (void)
{

	sMagField field = {0, 0, 0};

	if (m_NumSample > 0)
	{
		m_BufferLock.get();
		field = m_SampleBuffer[m_BufferOutIdx++];
		m_BufferOutIdx &= (LDSM9DS1_MAG_CIRC_BUFFER_SIZE-1);
		m_NumSample --;
		if (m_NumSample < 0)
		{
			//Not possible ...
			m_NumSample = 0;
		}
		m_BufferLock.release();
		//printf ("compass read out: X: %f | Y: %f | Z: %f \n", field.x, field.y, field.z);
	}

	return field;
}

void CCompass_LSM9DS1::dataReady (void)
{
	uint8_t buffer [6];
	int16_t sx, sy, sz;
	float32_t fx,fy,fz;
	sMagField field;
	if (m_I2Cbus == NULL)
	{
		return;
	}
	//Get data from the LSM9DS1 magnetometer.
	if (6 == m_I2Cbus->read(LSM9DS1_MAG_I2C_ADD, OUT_X_L_M, buffer,6))
	{
		sx = ((buffer[1])<< 8) + buffer[0] ;
		sy = ((buffer[3])<< 8) + buffer[2];
		sz = (buffer[5]<< 8) + buffer[4];


		field.x = (((float32_t) sy / 1500.0f * m_CompCalScale.x)-m_CompCalOffset.x);
		field.y = (((float32_t)-sx / 1500.0f * m_CompCalScale.y)-m_CompCalOffset.y);
		field.z = (((float32_t) sz / 1500.0f * m_CompCalScale.z)-m_CompCalOffset.z);

		field.x = (((float32_t) sy / 1500.0f) - m_CompCalOffset.x) * m_CompCalScale.x;
		field.y = (((float32_t)-sx / 1500.0f) - m_CompCalOffset.y) * m_CompCalScale.y;
		field.z = (((float32_t) sz / 1500.0f) - m_CompCalOffset.z) * m_CompCalScale.z;

		//float32_t norm = sqrtf(field.x * field.x + field.y * field.y + field.z * field.z);

		//printf ("compass read out: norm: %f | X: %f | Y: %f | Z: %f\n",norm, field.x, field.y, field.z);


		m_BufferLock.get();
		//We need to upsample to 119Hz to match Accelerometer and Gyro sample rate.
		//We are not going to perform a correct upsampling as the ratio is not easy.
		//instead we are going to copy the same value 3 times until we reach 117, and
		//then only 2 times to get 119 values from only 40 input values.

		uint32_t numCpy = (m_InputSampCnt<39)?3:2;

		for (uint32_t i = 0; i < numCpy; i++)
		{
			m_SampleBuffer[m_BufferInIdx++] = field;
			m_BufferInIdx &= (LDSM9DS1_MAG_CIRC_BUFFER_SIZE-1);

			m_NumSample++;
			if (m_NumSample > LDSM9DS1_MAG_CIRC_BUFFER_SIZE)
			{
				m_NumSample = LDSM9DS1_MAG_CIRC_BUFFER_SIZE;
				m_BufferOutIdx++;
				m_BufferOutIdx &= (LDSM9DS1_MAG_CIRC_BUFFER_SIZE-1);
				//printf("[LSM9DS1 MAG] Overflow detected on circular buffer !\n");
			}
		}
		m_InputSampCnt++;
		if (m_InputSampCnt >= 40)
		{
			m_InputSampCnt = 0;
		}
		m_BufferLock.release();

		//printf ("compass read out: X: %f/%f/%f/%d | Y: %f/%f/%f/%d | Z: %f/%f/%f/%d | heading: %f | norm: %f\n", fx, m_MaxX, m_MinX, m_CalX, fy, m_MaxY, m_MinY, m_CalY, fz, m_MaxZ, m_MinZ, m_CalZ, heading, norm);

	}
}

