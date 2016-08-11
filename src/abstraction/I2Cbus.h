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

#include "YapiBotTypes.h"

class CI2Cbus {
public:
	CI2Cbus(int32_t bus);
	~CI2Cbus();

	int32_t write (int8_t add, uint8_t * buff, uint32_t size);
	int32_t read (int8_t add, uint8_t* buff, uint32_t size);
	int32_t write (int8_t add, uint8_t subAdd,  uint8_t * buff, uint32_t size);
	int32_t read (int8_t add, uint8_t subAdd,  uint8_t * buff, uint32_t size);

private:
	int m_Handle;

};

#endif /* I2CBUS_H_ */
