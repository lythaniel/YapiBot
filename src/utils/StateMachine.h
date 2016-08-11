/*
 * StateMachine.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "Event.h"
#include <queue>
#include <map>
#include "Mutex.h"
#include "Thread.h"
#include "Semaphore.h"

#define STM_NOSTATE -1

typedef int32_t StateId_t;
typedef struct {
	Event_t evt;
	int32_t data1;
	void * data2;
} StmEvent_t;

class CTransition;

class CState {
public:
	CState(StateId_t id, const int8_t * StateName);
	virtual ~CState();
	virtual void onEnter(void) = 0;
	virtual CTransition * onEvent (Event_t evt, int32_t data1, void * data2);
	virtual void onExit(void) {}

	virtual void addTransition (CTransition * trans);

	virtual const int8_t * getName (void) {return m_Name;}
	virtual StateId_t getId (void) {return m_Id;}

private:
	typedef std::vector<CTransition *> TransVect_t;
	TransVect_t m_TransList;
	const int8_t * m_Name;
	StateId_t m_Id;
	StmEvent_t m_TransEvent;
};

class CTransition {
public:
	CTransition(StateId_t to, Event_t event) : m_To(to), m_Event(event) {}
	~CTransition();
	Event_t getEvent (void) {return m_Event;}
	StateId_t getNextState (void) {return m_To;}
private:
	StateId_t m_To;
	Event_t m_Event;
};


class CStateMachine {
public:
	CStateMachine();
	~CStateMachine();

	void start(void);
	void stop (void);
	void pause (void);

	void addState (StateId_t id, CState * state, bool initstate = false);
	void addTransition (StateId_t id,  CTransition * trans);
	StateId_t getCurrentState (void);
	const int8_t * getStateName (StateId_t id);

	void rcvEvent (Event_t evt, int32_t data1, void * data2);

	void run (void *);


private:
	typedef std::map <StateId_t, CState *> StateMap_t;
	StateMap_t m_StateMap;

	typedef std::queue<StmEvent_t *> EventQueue_t;
	EventQueue_t m_EventQueue;

	StateId_t m_CurrentState;
	StateId_t m_InitState;

	bool	m_Running;

	CMutex m_Lock;
	CMutex m_EventQueueLock;
	CThread * m_Thread;
	CSemaphore m_SemEvtRec;

};

#endif /* STATEMACHINE_H_ */
