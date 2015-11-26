/*
 * StateMachine.cpp
 *
 *  Created on: 13 déc. 2014
 *      Author: lythaniel
 */

#include "StateMachine.h"
#include "EventObserver.h"
#include <stdio.h>


CState::CState(StateId_t id, const char * StateName) :
m_Id (id),
m_Name(StateName)
{

}

CState::~CState()
{
	TransVect_t::iterator it;
	for (it = m_TransList.begin(); it != m_TransList.end(); it++)
	{
		delete *it;
		m_TransList.erase (it);
	}
}


CTransition * CState::onEvent (Event_t evt, int data1, void * data2)
{
	TransVect_t::iterator it;
	for (it = m_TransList.begin(); it != m_TransList.end(); it++)
	{
		if (evt == (*it)->getEvent())
		{
			m_TransEvent.evt = evt;
			m_TransEvent.data1 = data1;
			m_TransEvent.data2 = data2;
			return *it;
		}
	}
	return NULL;
}



void CState::addTransition (CTransition * trans)
{
	TransVect_t::iterator it;
	Event_t evt = trans->getEvent();
	for (it = m_TransList.begin(); it != m_TransList.end(); it++)
	{

		if (evt == (*it)->getEvent())
		{
			delete *it;
			m_TransList.erase (it);
			fprintf(stdout,"Warning: for state %s transition %d already exist. previous one deleted !\n");
		}
	}
	m_TransList.push_back(trans);
}



CStateMachine::CStateMachine():
m_CurrentState(STM_NOSTATE),
m_InitState (STM_NOSTATE),
m_Running(false),
m_Thread(NULL)
{
	m_Thread = new CThread ();
	m_Thread->regThreadProcess(this, &CStateMachine::run);
	m_Thread->start();
	CEventObserver::getInstance()->registerOnEvent(EVENT_MASK_ALL,this,&CStateMachine::rcvEvent);
}

CStateMachine::~CStateMachine()
{
	StateMap_t::iterator it;
	for (it = m_StateMap.begin(); it != m_StateMap.end(); it++)
	{
		delete *it->second;
		m_StateMap.erase (it);
	}
}

void CStateMachine::start(void)
{
	m_Lock.get();
	if (m_InitState != STM_NOSTATE)
	{
		if (m_CurrentState == STM_NOSTATE)
		{
			m_CurrentState = m_InitState;
			m_StateMap[m_CurrentState]->onEnter();
		}
		m_Running = true;
	}
	else
	{
		fprintf (stdout, "STM error: Cannot start STM as initial state is not defined !\n");
	}
	m_Lock.release();
}
void CStateMachine::stop (void)
{
	m_Lock.get();
	m_CurrentState = STM_NOSTATE;
	m_Running = false;
	m_Lock.release();

}
void CStateMachine::pause (void)
{
	m_Lock.get();
	m_Running = false;
	m_Lock.release();
}

void CStateMachine::addState (StateId_t id, CState * state, bool initstate)
{

	StateMap_t::iterator it;

	m_Lock.get();
	it = m_StateMap.find(id);

	if (it != m_StateMap.end())
	{
		fprintf (stdout, "STM error: State already exists. will not be added again !\n");

	}
	else
	{
		m_StateMap.insert(std::pair<StateId_t, CState *>(id,state));
		if (initstate)
		{
			m_InitState = id;
		}

	}
	m_Lock.release();

}
void CStateMachine::addTransition (StateId_t id, CTransition * trans)
{
	StateMap_t::iterator it;

	m_Lock.get();
	it = m_StateMap.find(id);

	if (it != m_StateMap.end())
	{
		StateId_t nextState = trans->getNextState();
		it = m_StateMap.find(nextState);
		if ((it != m_StateMap.end())||(nextState == STM_NOSTATE))
		{
			m_StateMap[id]->addTransition(trans);
		}
		else
		{
			fprintf (stdout, "STM warning: Trying to add a transition to an unknown state !\n");
		}
	}
	else
	{
		fprintf (stdout, "STM warning: Could not add transition to non existing state !\n");
	}
	m_Lock.release();

}
StateId_t CStateMachine::getCurrentState (void)
{
	return m_CurrentState;
}

void CStateMachine::rcvEvent (Event_t evt, int data1, void * data2)
{
	m_EventQueueLock.get();
	if (m_Running)
	{
		StmEvent_t * event = new  StmEvent_t;
		event->evt = evt;
		event->data1 = data1;
		event->data2 = data2;
		m_EventQueue.push(event);
		m_SemEvtRec.post();
	}
	m_EventQueueLock.release();
}

void CStateMachine::run(void *)
{
	StmEvent_t * event;
	CTransition * trans;


	while(1)
	{
		m_SemEvtRec.wait();
		m_EventQueueLock.get();
		while (!m_EventQueue.empty())
		{
			event = m_EventQueue.pop();
			if ((m_CurrentState != STM_NOSTATE)&&(m_Running == true))
			{
				m_EventQueueLock.release();
				m_Lock.get();
				trans = m_StateMap[m_CurrentState]->onEvent(event->evt,event->data1,event->data2);
				if (trans != NULL)
				{
					m_StateMap[m_CurrentState]->onExit();
					m_CurrentState = trans->getNextState();
					m_StateMap[m_CurrentState]->onEnter();

				}
				m_Lock.release();

				m_EventQueueLock.get();
			}
			delete event;
		}
		m_EventQueueLock.release();

	}

}
