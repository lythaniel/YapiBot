/*
 * I2Cbus.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef I2CBUS_H_
#define I2CBUS_H_

class CI2Cbus {
public:
	CI2Cbus(int bus);
	~CI2Cbus();

	int write (char add, unsigned char * buff, unsigned int size);
	int read (char add, unsigned char* buff, unsigned int size);
	int write (char add, unsigned char subAdd,  unsigned char * buff, unsigned int size);
	int read (char add, unsigned char subAdd,  unsigned char * buff, unsigned int size);

private:
	int m_Handle;

};

#endif /* I2CBUS_H_ */
