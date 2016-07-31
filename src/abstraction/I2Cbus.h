/*
 * I2Cbus.h
 *
 *  Created on: 17 déc. 2014
 *      Author: lythaniel
 */

#ifndef I2CBUS_H_
#define I2CBUS_H_

class CI2Cbus {
public:
	CI2Cbus(int bus);
	~CI2Cbus();

	int write (char add, char * buff, unsigned int size);
	int read (char add, char * buff, unsigned int size);

private:
	int m_Handle;

};

#endif /* I2CBUS_H_ */
