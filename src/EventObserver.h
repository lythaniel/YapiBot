/*
 * EventObserver.h
 *
 *  Created on: 15 févr. 2015
 *      Author: lythaniel
 */

#ifndef EVENTOBSERVER_H_
#define EVENTOBSERVER_H_

#include "Singleton.h"
#include "Event.h"
#include "CallBack.h"
#include <vector>
#include <utility>

class CEventObserver: public CSingleton<CEventObserver> {
public:
	CEventObserver();
	~CEventObserver();

	void notify (Event_t event, int data1 = 0,  void * data2 = NULL);


	template <class C>
	void registerOnEvent (EventMask_t mask, C* instance, void (C::*method)(Event_t event, int data1,  void * data2)){
		Callback3base<Event_t, int, void *> * cb = new Callback3<C,Event_t,int,void *> (instance, method);
		m_cbList.push_back(cbPair_t(mask,cb));
	}


private:
	typedef std::pair <EventMask_t, Callback3base<Event_t, int, void *> *> cbPair_t;
	typedef std::vector <cbPair_t> cbVector_t;

	cbVector_t  m_cbList;

};

#endif /* EVENTOBSERVER_H_ */
