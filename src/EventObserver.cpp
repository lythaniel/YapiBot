/*
 * EventObserver.cpp
 *
 *  Created on: 15 févr. 2015
 *      Author: lythaniel
 */

#include "EventObserver.h"

CEventObserver::CEventObserver() {

}

CEventObserver::~CEventObserver() {

}

void CEventObserver::notify (Event_t event, int data1,  void * data2)
{
	cbVector_t::iterator it;

	for (it = m_cbList.begin(); it != m_cbList.end(); it++)
	{
		if (event & it->first) {
			it->second->trigger(event, data1, data2);
		}

	}
}

