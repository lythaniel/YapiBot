/*
 * Sampler.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#include <semaphore.h>
#include <time.h>

class CSampler {
public:
	CSampler(unsigned int period);
	~CSampler();

	void wait (void);

private:
	sem_t m_Sem;
	unsigned int m_Period;
	long long m_NSecPeriod;
	int m_SecPeriod;
	timespec m_Time;

};

#endif /* SAMPLER_H_ */
