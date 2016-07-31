/*
 * I2Cbus.cpp
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#include "I2Cbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

CI2Cbus::CI2Cbus(int bus) {
	char filename [] = "/dev/i2c-x";
	filename[9] = 0x30 + bus;
	m_Handle = open (filename, O_RDWR);
	if (m_Handle < 0)
	{
		printf ("failed to open i2c bus (%d): %s\n",m_Handle,filename);
	}


}

CI2Cbus::~CI2Cbus() {
	close (m_Handle);
}

int CI2Cbus::write (char add, char * buff, unsigned int size)
{
	int ret = 0;
	if (ioctl(m_Handle, I2C_SLAVE, add) < 0)
	{
		printf ("Could not set I2C slave address !\n");
	}
	else
	{
		ret = ::write (m_Handle, buff, size);
	}

	return ret;
}

int CI2Cbus::read (char add, char * buff, unsigned int size)
{
	int ret = 0;
	if (ioctl(m_Handle, I2C_SLAVE, add) < 0)
	{
		printf ("Could not set I2C slave address !\n");
	}
	else
	{
		ret = ::read (m_Handle, buff, size);
	}

	return ret;
}

