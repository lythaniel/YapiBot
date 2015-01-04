/*
 * RangeFinder.cpp
 *
 *  Created on: 1 janv. 2015
 *      Author: lythaniel
 */

#include "RangeFinder.h"

#define ADC_I2C_ADD 0x6a

CRangeFinder::CRangeFinder() :
m_I2Cbus (NULL)
{

}

CRangeFinder::~CRangeFinder()
{

}

void CRangeFinder::setI2Cbus (CI2Cbus * bus)
{
	char confreg = (0x00 << 5) | (1 << 4) | (0x2 << 2) | (0x00); //Channel 1 / Continuous mode / 15 SPS(16bits) / x1
	if (bus != NULL)
	{
		m_I2Cbus = bus;
		m_I2Cbus->write(ADC_I2C_ADD, &confreg, 1);
	}
}

int CRangeFinder::getRange(void)
{
	int range = -1;
	int readout;
	float frng;
	char buff[3];
	if (m_I2Cbus)
	{
		m_I2Cbus->read(ADC_I2C_ADD, buff, 3);
		readout = (buff[0] << 8) + (buff[1]);
		//printf ("Rangefinder readout: %d, confreg 0x%x\n",range,buff[2]);
		range = 375000 / ((float)readout - 500);
	}
	return range;
}

