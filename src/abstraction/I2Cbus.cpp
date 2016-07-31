/*
 * I2Cbus.cpp
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#include "I2Cbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <pigpiod_if2.h>

CI2Cbus::CI2Cbus(int bus) {
	char filename [] = "/dev/i2c-x";
	filename[9] = 0x30 + bus;
	m_Handle = open (filename, O_RDWR);
	/*m_Pi = pigpio_start(NULL,NULL);
	if (m_Pi < 0)
	{
		fprintf(stderr, "[I2C]Error Could not initialize pigpio !");
		return;
	}*/

	if (m_Handle < 0)
	{
		printf ("failed to open i2c bus (%d): %s\n",m_Handle,filename);
	}


}

CI2Cbus::~CI2Cbus() {
	close (m_Handle);
}

int CI2Cbus::write (char add, unsigned char * buff, unsigned int size)
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

int CI2Cbus::read (char add, unsigned char * buff, unsigned int size)
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

int CI2Cbus::write (char add, unsigned char subAdd, unsigned char * buff, unsigned int size)
{
	int ret = 0;
	i2c_msg msgs [2];
	i2c_rdwr_ioctl_data ioctl_data;
	ioctl_data.msgs = &msgs[0];
	ioctl_data.nmsgs = 2;

	msgs[0].addr = add;
	msgs[0].flags = 0;
	msgs[0].buf = &subAdd;
	msgs[0].len = 1;

	msgs[1].addr = add;
	msgs[1].flags = I2C_M_NOSTART;
	msgs[1].buf = buff;
	msgs[1].len = size;

	ret = ioctl(m_Handle, I2C_RDWR, &ioctl_data);

	return ret;
}

int CI2Cbus::read (char add, unsigned char subAdd, unsigned char * buff, unsigned int size)
{
	int ret = 0;
	i2c_msg msgs [2];
	i2c_rdwr_ioctl_data ioctl_data;
	ioctl_data.msgs = &msgs[0];
	ioctl_data.nmsgs = 2;

	msgs[0].addr = add;
	msgs[0].flags = 0;
	msgs[0].buf = &subAdd;
	msgs[0].len = 1;

	msgs[1].addr = add;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = buff;
	msgs[1].len = size;

	ret = ioctl(m_Handle, I2C_RDWR, &ioctl_data);
	if (ret == 2)
	{
		ret = msgs[1].len;
	}
	else
	{
		ret = -1; //Something wrong.
	}

	return ret;
}

