/*
 * Network.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#include "Network.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "Thread.h"
#include "Mutex.h"
#include "Semaphore.h"
#include "EventObserver.h"
#include "Settings.h"
#include "Utils.h"


#define MAXPENDING 1    /* Max connection requests */
#define BUFFSIZE 32

#define VIDEOPORT 9999
#define CMDPORT 9998

CNetwork::CNetwork() :
m_VideoServerSock(-1),
m_VideoClientSock(-1),
m_CmdServerSock(-1),
m_CmdClientSock(-1),
m_pRxCb (NULL),
m_VideoClientConnected(false),
m_CmdClientConnected(false),
m_pRxCmdThread(NULL)
{
	m_VideoPort = CSettings::getInstance()->getInt("NETWORK","Video port",VIDEOPORT);
	m_CmdPort = CSettings::getInstance()->getInt("NETWORK","Command port",CMDPORT);

	m_pVideoServerThread = new CThread(NULL);
	m_pVideoServerThread->regThreadProcess(this,&CNetwork::VideoServerThread);
	m_pVideoSockMutex = new CMutex();
	m_pVideoClientDisconnected = new CSemaphore(1);

	m_pCmdServerThread = new CThread(NULL);
	m_pCmdServerThread->regThreadProcess(this,&CNetwork::CmdServerThread);
	m_pCmdSockMutex = new CMutex();
	m_pCmdClientDisconnected = new CSemaphore(1);

	m_pCmdTxBuffer = new char [sizeof(YapiBotHeader_t) + YAPIBOT_MAX_PL_SIZE];
	m_pCmdRxBuffer = new char [sizeof(YapiBotHeader_t) + YAPIBOT_MAX_PL_SIZE];

	m_pCmdTxHeader = (YapiBotHeader_t *)&m_pCmdTxBuffer[0];
	m_pCmdTxPayload = m_pCmdTxBuffer + sizeof(YapiBotHeader_t);
	m_pCmdTxHeader->magicNumber = YAPIBOT_MAGIC_NUMBER;

}
CNetwork::~CNetwork()
{
	//TO DO: CLEAN UP !
	if (m_pCmdTxBuffer != NULL)
	{
		delete [] m_pCmdTxBuffer;
	}
	if (m_pCmdRxBuffer != NULL)
	{
		delete [] m_pCmdRxBuffer;
	}
}

void CNetwork::start(void)
{
	m_pVideoServerThread->start();
	m_pCmdServerThread->start();
	
}
void CNetwork::stop (void)
{
	
}

void CNetwork::sendCmdPck (YapiBotCmd_t id, unsigned char * buffer, unsigned int size)
{
	if (m_pCmdSockMutex->get(MUTEX_TIMEOUT_DONTWAIT))
		{
			if (m_CmdClientConnected)
			{
				m_pCmdTxHeader->id = id;
				if (size > YAPIBOT_MAX_PL_SIZE)
				{
					fprintf (stderr, "[NETWORK] Error max pl size reached for Tx payload, payload will be truncated (id = %d, size = %d)", id,size);
					size = YAPIBOT_MAX_PL_SIZE;
				}
				m_pCmdTxHeader->payloadSize = size;
				memcpy (m_pCmdTxPayload, buffer, size);

				int ret = send(m_CmdClientSock, m_pCmdTxBuffer,sizeof(YapiBotHeader_t) + m_pCmdTxHeader->payloadSize, 0);
				if (ret == -1)
				{
					m_CmdClientSock = -1;
					m_CmdClientConnected = false;
					m_pCmdClientDisconnected->post();
					CEventObserver::getInstance()->notify(NetCmdClientDisconnected);
					fprintf(stdout,"Command client disconnected\n");
				}
			}
			m_pCmdSockMutex->release();
		}
}

void CNetwork::sendVideoPacket (unsigned char * buffer, unsigned int size)
{
	if (m_pVideoSockMutex->get(MUTEX_TIMEOUT_DONTWAIT))
	{
		if (m_VideoClientConnected)
		{
			int ret = send(m_VideoClientSock, buffer, size, 0);
			if (ret == -1)
			{
				m_VideoClientSock = -1;
				m_VideoClientConnected = false;
				m_pVideoClientDisconnected->post();
				CEventObserver::getInstance()->notify(NetVideoClientDisconnected);
				fprintf(stdout,"Video client disconnected\n");
			}
		}
		m_pVideoSockMutex->release();
	}
}

void CNetwork::VideoServerThread (void *)
{
	struct sockaddr_in serveraddr, clientaddr;
	if ((m_VideoServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stdout,"Failed to create socket\n");
	}
	memset(&serveraddr, 0, sizeof(serveraddr));       /* Clear struct */
	serveraddr.sin_family = AF_INET;                  /* Internet/IP */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	serveraddr.sin_port = htons(m_VideoPort);       /* server port */
	
	
	
	/* Bind the server socket */
	if (bind(m_VideoServerSock, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) {
		fprintf(stdout,"Failed to bind the server socket\n");
	}
	/* Listen on the server socket */
	if (listen(m_VideoServerSock, MAXPENDING) < 0) {
		fprintf(stdout,"Failed to listen on server socket\n");
	}
	CEventObserver::getInstance()->notify(NetVideoServerReady);

    /* Run until cancelled */
	while (1) {
		unsigned int clientlen = sizeof(clientaddr);
		/* Wait for client connection */
		m_pVideoSockMutex->get();
		if ((m_VideoClientSock = accept(m_VideoServerSock, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
			fprintf(stdout,"Failed to accept client connection\n");
			sleep(1);
			m_pVideoSockMutex->release();
		}
		else {
			fprintf(stdout, "Video client connected: %s\n", inet_ntoa(clientaddr.sin_addr));
			m_VideoClientConnected = true;
			m_pVideoSockMutex->release();
			CEventObserver::getInstance()->notify(NetVideoClientConnected);
			m_pVideoClientDisconnected->wait();
		}
		

	}
	 
}


void CNetwork::CmdServerThread (void *)
{
	struct sockaddr_in serveraddr, clientaddr;
	if ((m_CmdServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stdout,"Failed to create socket\n");
	}
	memset(&serveraddr, 0, sizeof(serveraddr));       /* Clear struct */
	serveraddr.sin_family = AF_INET;                  /* Internet/IP */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	serveraddr.sin_port = htons(m_CmdPort);       /* server port */



	/* Bind the server socket */
	if (bind(m_CmdServerSock, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) {
		fprintf(stdout,"Failed to bind the server socket\n");
	}
	/* Listen on the server socket */
	if (listen(m_CmdServerSock, MAXPENDING) < 0) {
		fprintf(stdout,"Failed to listen on server socket\n");
	}
	CEventObserver::getInstance()->notify(NetCmdServerReady);

    /* Run until cancelled */
	while (1) {
		unsigned int clientlen = sizeof(clientaddr);
		/* Wait for client connection */
		m_pCmdSockMutex->get();
		if ((m_CmdClientSock = accept(m_CmdServerSock, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
			fprintf(stdout,"Failed to accept client connection\n");
			sleep(1);
			m_pCmdSockMutex->release();
		}
		else {
			fprintf(stdout, "Cmd client connected: %s\n", inet_ntoa(clientaddr.sin_addr));
			m_CmdClientConnected = true;
			if (m_pRxCmdThread != NULL)
			{
				m_pRxCmdThread->kill();
				delete m_pRxCmdThread;
			}
			m_pRxCmdThread = new CThread();
			m_pRxCmdThread->regThreadProcess(this,&CNetwork::RxCmdThread);
			m_pRxCmdThread->start();

			m_pCmdSockMutex->release();
			CEventObserver::getInstance()->notify(NetCmdClientConnected);
			m_pCmdClientDisconnected->wait();
		}
	}

}


void CNetwork::RxCmdThread (void *)
{
	YapiBotHeader_t * header;
	char payload[YAPIBOT_MAX_PL_SIZE];
	unsigned int size_to_copy = 0;
	unsigned int payloadIdx = 0;
	unsigned int len;
	YapiBotCmd_t id;

	while (1)
	{
		//if (m_pCmdSockMutex->get())
		{
			if (m_CmdClientConnected)
			{
				int ret = recv(m_CmdClientSock, m_pCmdRxBuffer, (sizeof(YapiBotHeader_t) + YAPIBOT_MAX_PL_SIZE), 0);
				if (ret == -1)
				{
					m_CmdClientSock = -1;
					m_CmdClientConnected = false;
					m_pCmdClientDisconnected->post();
					CEventObserver::getInstance()->notify(NetCmdClientDisconnected);
					fprintf(stdout,"Command client disconnected\n");
				}
				else {
					len = ret;
					unsigned int idx = 0;
					while (len > 0)
					{
						if (size_to_copy > 0) //We still have payload to copy.
						{
							unsigned int copysz = (size_to_copy<len)?size_to_copy:len;
							if ((copysz + payloadIdx) > YAPIBOT_MAX_PL_SIZE)
							{
								//Something is very wrong here.
								copysz = YAPIBOT_MAX_PL_SIZE - payloadIdx;
								fprintf (stderr, "[NETWORK] Error during payload copy, payload will be truncated");
							}
							memcpy(&payload[payloadIdx],&m_pCmdRxBuffer[idx],copysz);
							len -= copysz;
							size_to_copy -= copysz;
							idx += copysz;
							payloadIdx += copysz;
							if ((size_to_copy == 0)||(payloadIdx == YAPIBOT_MAX_PL_SIZE))
							{
								//Trigger callback.
								if (m_pRxCb != NULL)
								{
									m_pRxCb->trigger(id, payload, payloadIdx);
								}
								size_to_copy = 0;
								payloadIdx = 0;
							}
						}
						else
						{
							header = (YapiBotHeader_t *)&m_pCmdRxBuffer[idx];
							if (header->magicNumber == YAPIBOT_MAGIC_NUMBER)
							{
								idx += sizeof (YapiBotHeader_t);
								len -= sizeof (YapiBotHeader_t);

								if (header->payloadSize > YAPIBOT_MAX_PL_SIZE)
								{
									fprintf (stderr, "[NETWORK] Error max pl size reached for Rx payload, payload will be truncated (id = %d, size = %d)", header->id,header->payloadSize);
									header->payloadSize = YAPIBOT_MAX_PL_SIZE;
								}
								size_to_copy = header->payloadSize;
								payloadIdx = 0;
								id = header->id;
								if (size_to_copy == 0)
								{
									//Trigger callback without payload.
									if (m_pRxCb != NULL)
									{
										m_pRxCb->trigger(id, NULL, payloadIdx);
									}
								}
							}
							else //We are not synchronized.
							{
								//Advance to the next byte.
								len -= 1;
								idx += 1;
							}
						}
					}
				}
			}
			//m_pCmdSockMutex->release();
		}

	}
}

