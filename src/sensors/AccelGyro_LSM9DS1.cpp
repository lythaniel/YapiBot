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
#include <pigpiod_if2.h>
#include <string.h>
#include <Settings.h>
#include <math.h>

#define INT1_AG_GPIO			26
#define INT2_AG_GPIO			19

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


#define CTRL_REG1_G_VAL			0x60	//Data rate = 119Hz / FS = 245dps / BW= 00
#define CTRL_REG2_G_VAL			0x00	//Filtering disabled.
#define CTRL_REG3_G_VAL			0x00	//Low power disabled, HPF disabled, HP cutoff 0000
#define ORIENT_CFG_G_VAL		0x00	//Default orientation.
#define CTRL_REG4_VAL			0x38	//gyro X,Y,Z enabled, latched interrupt disabled, 4D position disabled.

#define CTRL_REG5_XL_VAL		0x38	//No decimation, x,y and z enabled.
#define CTRL_REG6_XL_VAL		0x60	//data rate 119Hz, full scale 2g, auto bandwidth
#define CTRL_REG7_XL_VAL		0x80	//High res enabled, filtering disabled.
#define CTRL_REG8_XL_VAL		0x04	//auto increment address active.

#define FIFO_CTRL_VAL			0xCF	//FIFO in continuous mode, Threshold = 16;
#define INT1_CTRL_VAL			0x08	//Int1 enabled on fifo threshold.
#define CTRL_REG9_VAL			0x02	//Fifo enabled

#define GYRO_FS					245.0f		// 245degree per second (must match value in CTRL_REG1_G_VAL)
#define ACCEL_FS				2.0f		// 2g (must match value in CTRL_REG6_XL_VAL)

#define TEMP_SENS_GYRO			0.05f
#define TEMP_SENS_ACCEL			0.0005f

static void lsm9ds1_int1_ag (int32_t pi, uint32_t gpio, uint32_t level, uint32_t tick, void * user)
{
	CAccelGyro_LSM9DS1 * lsm9ds1 = (CAccelGyro_LSM9DS1 *) user;
	lsm9ds1->fifoReady ();
}

CAccelGyro_LSM9DS1::CAccelGyro_LSM9DS1() :
m_Initialized(false),
m_Pi(-1),
m_NumAccelSamples(0),
m_NumAngRateSamples(0),
m_AccelBufferInIdx(0),
m_AngRateBufferInIdx(0),
m_AccelBufferOutIdx(0),
m_AngRateBufferOutIdx(0)
{

	memset (m_RawSamples,0x00,sizeof (m_RawSamples));
	memset (m_AccelBuffer,0x00,sizeof (m_AccelBuffer));
	memset (m_AngRateBuffer,0x00,sizeof (m_AngRateBuffer));

	m_AccelCalOffset.x = 0;
	m_AccelCalOffset.y = 0;
	m_AccelCalOffset.z = 0;
	m_AccelCalScale.x = 1.0f;
	m_AccelCalScale.y = 1.0f;
	m_AccelCalScale.z = 1.0f;

	m_GyroCalOffset.x = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalOffsetX",0.0f);
	m_GyroCalOffset.y = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalOffsetY",0.0f);
	m_GyroCalOffset.z = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalOffsetZ",0.0f);

	m_GyroCalScale.x = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalScaleX",1.0f);
	m_GyroCalScale.y = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalScaleY",1.0f);
	m_GyroCalScale.z = CSettings::getInstance()->getFloat("GYRO LSM9DS1","CalScaleZ",1.0f);

	m_AccelCalOffset.x = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalOffsetX",0.0f);
	m_AccelCalOffset.y = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalOffsetY",0.0f);
	m_AccelCalOffset.z = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalOffsetZ",0.0f);

	m_AccelCalScale.x = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalScaleX",1.0f);
	m_AccelCalScale.y = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalScaleY",1.0f);
	m_AccelCalScale.z = CSettings::getInstance()->getFloat("ACCEL LSM9DS1","CalScaleZ",1.0f);


}

CAccelGyro_LSM9DS1::~CAccelGyro_LSM9DS1()
{
	callback_cancel(m_IntCallbackId[0]);
	pigpio_stop(m_Pi);
}

extern int32_t pigpio;
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


		//Connect the interrupt handler.
		if (m_Initialized == false)
		{
			//Temporary until proper GPIO interface.
			m_Pi = pigpio;//pigpio_start(NULL,NULL); //local gpio
			set_mode (m_Pi, INT1_AG_GPIO, PI_INPUT);
			m_IntCallbackId[0] = callback_ex(m_Pi, INT1_AG_GPIO, RISING_EDGE, lsm9ds1_int1_ag, (void *)this);



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

			//Configure FIFO
			buff[0] = FIFO_CTRL;
			buff[1] = FIFO_CTRL_VAL;
			CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 2);
			buff[0] = CTRL_REG9;
			buff[1] = CTRL_REG9_VAL;
			CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 2);

			//configure interrupt 1
			buff[0] = INT1_CTRL;
			buff[1] = INT1_CTRL_VAL;
			CAccelerometer::m_I2Cbus->write(LSM9DS1_GXL_I2C_ADD,buff, 2);

			//Flush the fifo.
			CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_X_L_G, (uint8_t*)m_RawSamples,32*12);

			m_Initialized = true;
		}


	}
}

sAccel CAccelGyro_LSM9DS1::getAccel (void)
{

	sAccel ret = {0.0f, 0.0f, 0.0f};

	//Check if we have any samples available.
	if (m_NumAccelSamples > 0)
	{
		//Lock the buffer
		m_BufferLock.get();
		//Retrieve sample from circular buffer & advance read index
		ret = m_AccelBuffer[m_AccelBufferOutIdx++];
		m_AccelBufferOutIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);
		//Decrement numbers of samples.
		m_NumAccelSamples--;
		//Release buffer.
		m_BufferLock.release();
	}
	//double norm = sqrt(ret.x * ret.x + ret.y * ret.y + ret.z * ret.z);
	//printf ("[LSM9DS1 XL] Acceleration: norm = %f, x= %f | y = %f | z = %f\n", norm, ret.x, ret.y, ret.z);

	return ret;

}

sAngularRate CAccelGyro_LSM9DS1::getAngularRate (void)
{
	sAngularRate ret = {0.0f, 0.0f, 0.0f};

	//Check if we have any samples available.
	if (m_NumAngRateSamples > 0)
	{
		//Lock the buffer
		m_BufferLock.get();
		//Retrieve sample from circular buffer & advance read index
		ret = m_AngRateBuffer[m_AngRateBufferOutIdx++];
		m_AngRateBufferOutIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);
		//Decrement numbers of samples.
		m_NumAngRateSamples--;
		//Release buffer.
		m_BufferLock.release();
	}
	//printf ("[LSM9DS1 G] Angular rate: x= %f | y = %f | z = %f\n", ret.x, ret.y, ret.z);
	//printf ("[LSM9DS1 G] average Angular rate: x= %f | y = %f | z = %f\n", m_AverageAngRate.x, m_AverageAngRate.y, m_AverageAngRate.z);



	return ret;


}

void CAccelGyro_LSM9DS1::startGyroCalibration(void)
{
	m_GyroCalOffset.x = 0.0f;
	m_GyroCalOffset.y = 0.0f;
	m_GyroCalOffset.z = 0.0f;

	m_GyroCalScale.x = 1.0f;
	m_GyroCalScale.y = 1.0f;
	m_GyroCalScale.z = 1.0f;
}

void CAccelGyro_LSM9DS1::startAccelCalibration(void)
{
	m_AccelCalOffset.x = 0.0f;
	m_AccelCalOffset.y = 0.0f;
	m_AccelCalOffset.z = 0.0f;

	m_AccelCalScale.x = 1.0f;
	m_AccelCalScale.y = 1.0f;
	m_AccelCalScale.z = 1.0f;

}

void CAccelGyro_LSM9DS1::stopGyroCalibration (sAngularRate offset, sAngularRate scale)
{
	m_GyroCalOffset = offset;
	m_GyroCalScale = scale;

	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalOffsetX",m_GyroCalOffset.x);
	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalOffsetY",m_GyroCalOffset.y);
	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalOffsetZ",m_GyroCalOffset.z);

	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalScaleX",m_GyroCalScale.x);
	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalScaleY",m_GyroCalScale.y);
	CSettings::getInstance()->setFloat("GYRO LSM9DS1","CalScaleZ",m_GyroCalScale.z);

}



void CAccelGyro_LSM9DS1::stopAccelCalibration (sAccel offset, sAccel scale)
{
	m_AccelCalOffset = offset;
	m_AccelCalScale = scale;

	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalOffsetX",m_AccelCalOffset.x);
	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalOffsetY",m_AccelCalOffset.y);
	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalOffsetZ",m_AccelCalOffset.z);

	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalScaleX",m_AccelCalScale.x);
	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalScaleY",m_AccelCalScale.y);
	CSettings::getInstance()->setFloat("ACCEL LSM9DS1","CalScaleZ",m_AccelCalScale.z);

}

void  CAccelGyro_LSM9DS1::fifoReady (void)
{
	uint8_t fifoSrc;
	uint8_t numSampleInFifo;
	int16_t temperature;
	float32_t tempCorrGyro;
	float32_t tempCorrAccel;

	if (CAccelerometer::m_I2Cbus != NULL)
	{
		//Read FIFO_SRC to know the number of samples available in the fifo.
		if (1 != CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, FIFO_SRC, &fifoSrc,1))
		{
			return;
		}
		//check for overflow in the fifo.
		if (fifoSrc & 0x40)
		{
			printf ("[LSM9DS1] Warning, overrun detected on fifo\n");
		}

		numSampleInFifo = fifoSrc & 0x3F;
		if (2 != CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_TEMP_L, (uint8_t*)&temperature,2))
		{
			return;
		}
		tempCorrAccel = (float32_t)temperature * TEMP_SENS_ACCEL / 16.0f;

		//read the fifo. It is possible to read everything with a single read. (LSM9DS1 datasheet ch 3.3 p21)
		if (12*numSampleInFifo != CAccelerometer::m_I2Cbus->read(LSM9DS1_GXL_I2C_ADD, OUT_X_L_G, (uint8_t*)m_RawSamples,numSampleInFifo*12))
		{
			return;
		}

		//Convert and push everything into the local fifo.
		m_BufferLock.get();
		for (int i = 0; i < (numSampleInFifo*6); i+=6)
		{

			//Convert rotation speed.
			m_AngRateBuffer[m_AngRateBufferInIdx].x =  (((float32_t)m_RawSamples[i + 1] * GYRO_FS / 32768.0f / 180.0f * M_PI * m_GyroCalScale.x)-m_GyroCalOffset.x);
			m_AngRateBuffer[m_AngRateBufferInIdx].y =  (((float32_t)m_RawSamples[i + 0] * GYRO_FS / 32768.0f / 180.0f * M_PI * m_GyroCalScale.y)-m_GyroCalOffset.y);
			m_AngRateBuffer[m_AngRateBufferInIdx].z =  (((float32_t)m_RawSamples[i + 2] * GYRO_FS / 32768.0f / 180.0f * M_PI * m_GyroCalScale.z)-m_GyroCalOffset.z);

			//m_AverageAngRate.x = m_AverageAngRate.x * 0.99f + (float32_t)m_RawSamples[i + 0] * 0.01f;
			//m_AverageAngRate.y = m_AverageAngRate.y * 0.99f + (float32_t)m_RawSamples[i + 1] * 0.01f;
			//m_AverageAngRate.z = m_AverageAngRate.z * 0.99f + (float32_t)m_RawSamples[i + 2] * 0.01f;

			//Advance write index.
			m_AngRateBufferInIdx++;
			m_AngRateBufferInIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);

			//Increase number of samples and check for overflow.
			m_NumAngRateSamples ++;
			if (m_NumAngRateSamples > SAMPLES_CIRC_BUFFER_SIZE)
			{
				m_NumAngRateSamples = SAMPLES_CIRC_BUFFER_SIZE;
				m_AngRateBufferOutIdx++;
				m_AngRateBufferOutIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);
				//printf("[LSM9DS1 GYRO] Overflow detected on circular buffer !\n");
			}

			//Convert acceleration.
			m_AccelBuffer[m_AccelBufferInIdx].x =  (((float32_t)m_RawSamples[i + 4] * ACCEL_FS / 32768.0f * m_AccelCalScale.x)-m_AccelCalOffset.x /*+ tempCorrAccel*/);
			m_AccelBuffer[m_AccelBufferInIdx].y =  (((float32_t)m_RawSamples[i + 3] * ACCEL_FS / 32768.0f * m_AccelCalScale.y)-m_AccelCalOffset.y /*+ tempCorrAccel*/);
			m_AccelBuffer[m_AccelBufferInIdx].z =  (((float32_t)m_RawSamples[i + 5] * ACCEL_FS / 32768.0f * m_AccelCalScale.z)-m_AccelCalOffset.z /*+ tempCorrAccel*/);

			//double norm = sqrt(m_AccelBuffer[m_AccelBufferInIdx].x * m_AccelBuffer[m_AccelBufferInIdx].x + m_AccelBuffer[m_AccelBufferInIdx].y * m_AccelBuffer[m_AccelBufferInIdx].y + m_AccelBuffer[m_AccelBufferInIdx].z * m_AccelBuffer[m_AccelBufferInIdx].z);
			//printf ("[LSM9DS1 XL] Acceleration: temp = %d, norm = %f, x= %f | y = %f | z = %f\n", temperature, norm, m_AccelBuffer[m_AccelBufferInIdx].x, m_AccelBuffer[m_AccelBufferInIdx].y, m_AccelBuffer[m_AccelBufferInIdx].z);


			//Advance write index.
			m_AccelBufferInIdx++;
			m_AccelBufferInIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);

			//Increase number of samples and check for overflow.
			m_NumAccelSamples ++;
			if (m_NumAccelSamples > SAMPLES_CIRC_BUFFER_SIZE)
			{
				m_NumAccelSamples = SAMPLES_CIRC_BUFFER_SIZE;
				m_AccelBufferOutIdx++;
				m_AccelBufferOutIdx &= (SAMPLES_CIRC_BUFFER_SIZE-1);
				//printf("[LSM9DS1 ACCEL] Overflow detected on circular buffer !\n");
			}

		}
		m_BufferLock.release();
	}
}

