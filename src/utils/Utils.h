/*
 * Utils.h
 *
 *  Created on: 7 août 2016
 *      Author: lythaniel
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <string.h>

class Utils
{
public:
	static int toInt (void * buff) {
		int ret;
		memcpy (&ret, buff, 4);
		return (ret);
	}

	static void fromInt (int val,void * buff) {
		memcpy(buff,&val,4);

	}

	static float toFloat (void * buff) {
		float ret;
		memcpy (&ret,buff,4);
		return (ret);
	}

	static void fromFloat (float val, void * buff) {
		memcpy(buff,&val,4);
	}

};



#endif /* UTILS_H_ */
