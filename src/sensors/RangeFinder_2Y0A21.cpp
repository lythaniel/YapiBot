/*
 * RangeFinder.cpp
 *
 *  Created on: 1 janv. 2015
 *      Author: lythaniel
 */

#include "RangeFinder_2Y0A21.h"

#define ADC_I2C_ADD 0x68

CRangeFinder_2Y0A21::CRangeFinder_2Y0A21()
{

}

CRangeFinder_2Y0A21::~CRangeFinder_2Y0A21()
{

}

void CRangeFinder_2Y0A21::setBus (CI2Cbus * bus)
{
	char confreg = (0x01 << 5) | (1 << 4) | (0x2 << 2) | (0x00); //Channel 2 / Continuous mode / 15 SPS(16bits) / x1
	if (bus != NULL)
	{
		m_I2Cbus = bus;
		m_I2Cbus->write(ADC_I2C_ADD, &confreg, 1);
	}
}

int CRangeFinder_2Y0A21::getRange(void)
{
	int range = -1;
	int readout;
	float frng;
	char buff[3];
	if (m_I2Cbus)
	{
		m_I2Cbus->read(ADC_I2C_ADD, buff, 3);
		readout = (buff[0] << 8) + (buff[1]);
		//printf ("Rangefinder readout: %d, confreg 0x%x\n",readout,buff[2]);
		range = (490512 / (readout - 508)) - 4;
		//printf ("Rangefinder readout: %d, range %d\n",readout,range);
	}
	return range;
}
