
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


#define MAXPENDING 1    /* Max connection requests */
#define BUFFSIZE 32

#define VIDEOPORT 9999
#define CMDPORT 9998

CNetwork::CNetwork() :
m_VideoServerSock(-1),
m_VideoClientSock(-1),
m_CmdServerSock(-1),
m_CmdClientSock(-1),
m_pEventCb(NULL),
m_pRxCb (NULL),
m_VideoClientConnected(false),
m_CmdClientConnected(false),
m_pRxCmdThread(NULL)
{
	m_pVideoServerThread = new CThread(NULL);
	m_pVideoServerThread->regThreadProcess(this,&CNetwork::VideoServerThread);
	m_pVideoSockMutex = new CMutex();
	m_pVideoClientDisconnected = new CSemaphore(1);

	m_pCmdServerThread = new CThread(NULL);
	m_pCmdServerThread->regThreadProcess(this,&CNetwork::CmdServerThread);
	m_pCmdSockMutex = new CMutex();
	m_pCmdClientDisconnected = new CSemaphore(1);


}
CNetwork::~CNetwork()
{
	//TO DO: CLEAN UP !
}

void CNetwork::start(void)
{
	m_pVideoServerThread->start();
	m_pCmdServerThread->start();
	
}
void CNetwork::stop (void)
{
	
}

void CNetwork::sendCmdPck (unsigned char * buffer, unsigned int size)
{
	if (m_pCmdSockMutex->get(MUTEX_TIMEOUT_DONTWAIT))
		{
			if (m_CmdClientConnected)
			{
				int ret = send(m_CmdClientSock, buffer, size, 0);
				if (ret == -1)
				{
					m_CmdClientSock = -1;
					m_CmdClientConnected = false;
					m_pCmdClientDisconnected->post();
					m_pEventCb->trigger(CmdClientDisconnected);
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
				if (m_pEventCb != NULL) {
					m_pEventCb->trigger(VideoClientDisconnected);
				}
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
	serveraddr.sin_port = htons(VIDEOPORT);       /* server port */
	
	
	
	/* Bind the server socket */
	if (bind(m_VideoServerSock, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) {
		fprintf(stdout,"Failed to bind the server socket\n");
	}
	/* Listen on the server socket */
	if (listen(m_VideoServerSock, MAXPENDING) < 0) {
		fprintf(stdout,"Failed to listen on server socket\n");
	}
	if (m_pEventCb != NULL)
	{
		m_pEventCb->trigger(VideoServerReady);
	}
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
			if (m_pEventCb != NULL)
			{
				m_pEventCb->trigger(VideoClientConnected);
			}
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
	serveraddr.sin_port = htons(CMDPORT);       /* server port */



	/* Bind the server socket */
	if (bind(m_CmdServerSock, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) {
		fprintf(stdout,"Failed to bind the server socket\n");
	}
	/* Listen on the server socket */
	if (listen(m_CmdServerSock, MAXPENDING) < 0) {
		fprintf(stdout,"Failed to listen on server socket\n");
	}
	if (m_pEventCb != NULL)
	{
		m_pEventCb->trigger(VideoServerReady);
	}
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
			if (m_pEventCb != NULL)
			{
				m_pEventCb->trigger(CmdClientConnected);
			}
			m_pCmdClientDisconnected->wait();
		}
	}

}


void CNetwork::RxCmdThread (void *)
{
	while (1)
	{
		//if (m_pCmdSockMutex->get())
		{
			if (m_CmdClientConnected)
			{
				int ret = recv(m_CmdClientSock, m_RxBuffer, CMDBUFFER_SIZE, 0);
				if (ret == -1)
				{
					m_CmdClientSock = -1;
					m_CmdClientConnected = false;
					m_pCmdClientDisconnected->post();
					if (m_pEventCb != NULL) {
						m_pEventCb->trigger(CmdClientDisconnected);
					}
					fprintf(stdout,"Command client disconnected\n");
				}
				else {
					if (m_pRxCb != NULL) {
						m_pRxCb->trigger(m_RxBuffer,ret);
					}
				}
			}
			m_pCmdSockMutex->release();
		}

	}
}

