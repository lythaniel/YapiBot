/*
 * Utils.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <string.h>

class Utils
{
public:
	static int32_t toInt (void * buff) {
		int32_t ret;
		memcpy (&ret, buff, 4);
		return (ret);
	}

	static void fromInt (int32_t val,void * buff) {
		memcpy(buff,&val,4);

	}

	static float32_t toFloat (void * buff) {
		float32_t ret;
		memcpy (&ret,buff,4);
		return (ret);
	}

	static void fromFloat (float32_t val, void * buff) {
		memcpy(buff,&val,4);
	}

};



#endif /* UTILS_H_ */
