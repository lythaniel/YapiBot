/*
 * Sampler.cpp
 *
 *  Created on: 20 déc. 2014
 *      Author: lythaniel
 */

#include "Sampler.h"
#include <stdio.h>

CSampler::CSampler(unsigned int period) :
m_Period(period)
{
	sem_init (&m_Sem, 0, 0);
	m_NSecPeriod = m_Period%1000;
	m_NSecPeriod *= 1000000;
	m_SecPeriod = m_Period / 1000;

	clock_gettime(CLOCK_REALTIME,&m_Time);

	m_Time.tv_sec += m_SecPeriod;
	m_Time.tv_nsec += m_NSecPeriod;
	if (m_Time.tv_nsec >= 1000000000)
	{
		m_Time.tv_sec ++;
		m_Time.tv_nsec -= 1000000000;
	}

	//printf("Sampler created with perdiod: %ds / %d ns", m_SecPeriod,m_NSecPeriod);
}

CSampler::~CSampler()
{
	sem_destroy(&m_Sem);
}

void CSampler::wait(void)
{
	sem_timedwait(&m_Sem, &m_Time);
	clock_gettime(CLOCK_REALTIME,&m_Time);
	m_Time.tv_sec += m_SecPeriod;
	m_Time.tv_nsec += m_NSecPeriod;
	if (m_Time.tv_nsec >= 1000000000)
	{
		m_Time.tv_sec ++;
		m_Time.tv_nsec -= 1000000000;
	}

}


