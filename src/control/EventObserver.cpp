/*
 * EventObserver.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "EventObserver.h"

CEventObserver::CEventObserver() {

}

CEventObserver::~CEventObserver() {

}

void CEventObserver::notify (Event_t event, int32_t data1,  void * data2)
{
	cbVector_t::iterator it;

	for (it = m_cbList.begin(); it != m_cbList.end(); it++)
	{
		if (event & it->first) {
			it->second->trigger(event, data1, data2);
		}

	}
}

