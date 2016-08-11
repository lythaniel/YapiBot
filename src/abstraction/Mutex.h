/*
 * Mutex.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include "YapiBotTypes.h"
#include <pthread.h>

#define MUTEX_TIMEOUT_DONTWAIT 0
#define MUTEX_TIMEOUT_FOREVER -1

class CMutex
{
public:
	CMutex ();
	~CMutex ();

	bool get(int32_t timeout = MUTEX_TIMEOUT_FOREVER);
	void release(void);

private:
	pthread_mutex_t m_Lock;
};



#endif /* MUTEX_H_ */
