/******************************************************************************
 * SYNOPTIC :   A Synergistic Protocol for Optical networks
 * Copyright (C) 2006 Venkatram Vishwanath, Jason Leigh
 * Electronic Visualization Laboratory,
 * University of Illinois at Chicago
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either Version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License along
 * with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Direct questions, comments etc to venkat@evl.uic.edu
 *****************************************************************************/

/* Notes */
/* One can evaluate the effects of block partitioning
* There is merit in making this partitioning Cache efficient
* instead of the current static partitioning
*/


#include "SON_paralleltcp.h"
#include <stdlib.h>
#include <limits.h>

namespace SON_PROTOCOL
{


/*
Client port is expected to be provided by the user, else it will be set to the
next available port greater than the connection port.
This might be an issue with firewalls.
*/


SON_paralleltcpServer :: SON_paralleltcpServer():
					m_state(PARALLELTCP_SERVERSTATE_UNDEF),
					m_sendBufSize(SON_PROTOCOL::SON_SENDBUFFER_SIZE_DEFAULT),
					m_recvBufSize(SON_PROTOCOL::SON_RECVBUFFER_SIZE_DEFAULT)
{
	m_bigB = SON_BigBrother::getObj();

	m_connServer = new SON_tcpServer(m_sendBufSize, m_recvBufSize);

	m_dataServer = new SON_tcpServer(m_sendBufSize, m_recvBufSize);
	
	m_state = PARALLELTCP_SERVERSTATE_CREATED;
}


SON_paralleltcpServer::SON_paralleltcpServer(int sendbufsize, int recvbufsize):
					m_state(PARALLELTCP_SERVERSTATE_UNDEF),
					m_sendBufSize(sendbufsize),
					m_recvBufSize(recvbufsize)
{
	m_bigB = SON_BigBrother::getObj();

	m_connServer = new SON_tcpServer(m_sendBufSize, m_recvBufSize);

	m_dataServer = new SON_tcpServer(m_sendBufSize, m_recvBufSize);
	
	m_state = PARALLELTCP_SERVERSTATE_CREATED;
}



SON_paralleltcpServer :: ~SON_paralleltcpServer()
{
	/* CHECK if state is CLOSED; If not call close first */
	if (m_state != PARALLELTCP_SERVERSTATE_CLOSED)
		this->close();

	/* Delete Dynamically allocated Port Strings and TCP Servers */
	
	if (m_dataServer)
	{
		delete m_dataServer;
		m_dataServer = 0;
	}

	if (m_connServer)
	{
		delete m_connServer;
		m_connServer = 0;
	}

}


int SON_paralleltcpServer::init(int connport)
{
	int retval = -1;
	
	retval = m_connServer->init(connport);
	if (-1  == retval)
	{
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
		return retval;
	}
	
	/* Assign a Client Port */
	retval = -1;
	for (int port = (connport + 1) ; 
		port <  (connport + SON_DATASERVER_PORTRANGE) ; port++)
	{
		retval = m_dataServer->init(port);
		if ( 0 == retval)
		{
			m_state = PARALLELTCP_SERVERSTATE_INITIALIZED;
			return 0;
		}	
	}
	
	m_state = PARALLELTCP_SERVERSTATE_ERROR;
	return retval;
}

int SON_paralleltcpServer::init(int connport, int dataport)
{	
	if ((0 >= connport) || ( 0 >= dataport))
		return -1;
	
	if (m_state == PARALLELTCP_SERVERSTATE_INITIALIZED)
		return 0;
		
	int retval = -1;
	
	/* initialize the connection/control server */
	SON_PRINT_DEBUG2( \
		" SON_paralleltcpServer :: trying to init For Conn Port %d \n", \
		connport);
	
	retval = m_connServer->init(connport);
	if (-1  == retval)
	{
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
		return retval;
	}
	
	/* initialize the data server */
	SON_PRINT_DEBUG2( \
		" SON_paralleltcpServer :: trying to init For Data Port %d \n", \
		dataport);
	
	retval = m_dataServer->init(dataport);
	if ( 0 == retval)
		m_state = PARALLELTCP_SERVERSTATE_INITIALIZED;
	else
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
	return retval;
}


int SON_paralleltcpServer :: init(char* connport)
{
	if ( 0 == connport)
		return -1;
		
	if (m_state == PARALLELTCP_SERVERSTATE_INITIALIZED)
		return 0;
			
	int retval = -1;
	
	retval = m_connServer->init(connport);
	if (-1 == retval)
	{
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
		return retval;
	}

	/* Assign a Client Port */
	retval = -1;
	for (int port = (atoi(connport) + 1) ; 
		port <  (atoi(connport) + SON_DATASERVER_PORTRANGE) ; port++)
	{
		retval = m_dataServer->init(port);
		if ( 0 == retval)
		{
			m_state = PARALLELTCP_SERVERSTATE_INITIALIZED;
			return 0;
		}	
	}
	
	m_state = PARALLELTCP_SERVERSTATE_ERROR;
	return retval;
}


int SON_paralleltcpServer :: init(char* connport, char* dataport)
{
	if ((0 == connport) || ( 0 == dataport))
		return -1;
	  
	if (m_state == PARALLELTCP_SERVERSTATE_INITIALIZED)
		return 0;
			
	int retval = -1;
	
	/* initialize the connection/control server */
	SON_PRINT_DEBUG2( \
		" SON_paralleltcpServer :: trying to init For Conn Port %s \n", \
		connport);
	
	retval = m_connServer->init(connport);
	if (-1  == retval)
	{
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
		return retval;
	}
	
	/* initialize the data server */
	SON_PRINT_DEBUG2( \
		" SON_paralleltcpServer :: trying to init For Data Port %s \n", \
		dataport);

	retval = m_dataServer->init(dataport);
	if ( 0 == retval)
		m_state = PARALLELTCP_SERVERSTATE_INITIALIZED;
	else
		m_state = PARALLELTCP_SERVERSTATE_ERROR;
		
	return retval;
}



int SON_paralleltcpServer :: close()
{
	int retval = -1;
	m_state = PARALLELTCP_SERVERSTATE_CLOSED;

	/* Close the Server Port */
	retval = m_connServer->close();
	retval = m_dataServer->close();

	return retval;
}



SON_paralleltcpClient* SON_paralleltcpServer::waitForNewConnection()
{
	if (-1 == this->__isInitialized())
		return 0;

	int retval = -1;
	SON_paralleltcpClient* ptcp_client;
	
	SON_tcpClient* client;
	client  = m_connServer->waitForNewConnection();
	
	if (client)
	{
		/* Read the Number of Sockets */
		int numsocks = 0;
		long long int size_numsocks =  sizeof(int);
		
		retval = client->read((char*)&numsocks, size_numsocks);
		if (-1 == retval)
		{
			client->close();
			delete client;
			client = 0 ;
		 
			return 0;
		}
		cout << "  Number of Sockets are " << numsocks << endl;
		
		/* Create a New PTCP client */
		ptcp_client =  new SON_paralleltcpClient;
		
		/* Send the ClientPort Information to the remote Client */
		long long int dataport_size = SON_STRING_SIZE_SMALL;
		
		if ( ( 0 == m_dataServer->getPort()) || 
			(-1 == client->write( m_dataServer->getPort(), dataport_size)) )
		{
			client->close();
			delete client;
			client = 0 ;
			
			delete ptcp_client;
			ptcp_client = 0;
			
			return 0;
		}

		ptcp_client->m_numSockets = numsocks;
		
		int connectedclients = 0;
		SON_tcpClient* cliobj;
		
		while ( connectedclients < numsocks)
		{			
			cliobj = m_dataServer->waitForNewConnection();
		
			if (cliobj)
			{
				/* Add the TCP client and socketID to parallel TCP Client List */
				ptcp_client->m_clientList.push_back(cliobj);
				ptcp_client->m_sockFd.push_back(cliobj->getSockFd());
			
				connectedclients++;
			}	
		}
		
		/* We have now received all the clients, close the control channel */
		client->close();
		delete client;
		client = 0;
		
		ptcp_client->__setMaxSockFd();
		ptcp_client->setState(PARALLELTCP_CLIENTSTATE_CONNECTED);
		ptcp_client->__setNonBlocking();
		
		return ptcp_client;
	}
	
	return 0;
}


SON_paralleltcpClient* SON_paralleltcpServer::
									checkForNewConnection(int timeoutinsecs)
{
	if (-1 == this->__isInitialized())
		return 0;

	int retval = -1;
	SON_paralleltcpClient* ptcp_client;
	
	SON_tcpClient* client;
	client  = m_connServer->checkForNewConnection(timeoutinsecs);
	
	if (client)
	{
		/* Read the Number of Sockets */
		int numsocks = 0;
		long long int size_numsocks =  sizeof(int);
		
		retval = client->read((char*)&numsocks, size_numsocks);
		if (-1 == retval)
		{
			client->close();
			delete client;
			client = 0 ;
		 
			return 0;
		}
		cout << "  Number of Sockets are " << numsocks << endl;
		
		/* Create a New PTCP client */
		ptcp_client =  new SON_paralleltcpClient;
		
		/* Send the ClientPort Information to the remote Client */
		long long int dataport_size = SON_STRING_SIZE_SMALL;
		
		if ( ( 0 == m_dataServer->getPort()) || 
			(-1 == client->write( m_dataServer->getPort(), dataport_size)) )
		{
			client->close();
			delete client;
			client = 0 ;
			
			delete ptcp_client;
			ptcp_client = 0;
			
			return 0;
		}

		ptcp_client->m_numSockets = numsocks;
		
		int connectedclients = 0;
		SON_tcpClient* cliobj;
		
		while ( connectedclients < numsocks)
		{			
			cliobj = m_dataServer->waitForNewConnection();
		
			if (cliobj)
			{
				/* Add the TCP client and socketID to parallel TCP Client List */
				ptcp_client->m_clientList.push_back(cliobj);
				ptcp_client->m_sockFd.push_back(cliobj->getSockFd());
			
				connectedclients++;
			}	
		}
		
		/* We have now received all the clients, close the control channel */
		client->close();
		delete client;
		client = 0;
		
		ptcp_client->__setMaxSockFd();
		ptcp_client->setState(PARALLELTCP_CLIENTSTATE_CONNECTED);
		ptcp_client->__setNonBlocking();
		
		return ptcp_client;
	}
	
	return 0;
}



void SON_paralleltcpServer :: setRecvBufSize (int size)
{
	m_recvBufSize = size;
	
	return m_dataServer->setRecvBufSize(size);
}


int SON_paralleltcpServer :: getRecvBufSize (void)
{	
	return m_dataServer->getRecvBufSize();
}


void SON_paralleltcpServer :: setSendBufSize (int size)
{
	m_sendBufSize = size;
	
	return m_dataServer->setSendBufSize(size);
}


int SON_paralleltcpServer :: getSendBufSize (void)
{
	return m_dataServer->getSendBufSize();
}



int SON_paralleltcpServer :: __isInitialized (void)
{
	int retval = -1;
	
	switch (this->m_state)
	{
		case  PARALLELTCP_SERVERSTATE_INITIALIZED:
			this->m_state = PARALLELTCP_SERVERSTATE_ACCEPTING;
			retval = 0;
			break;

		case  PARALLELTCP_SERVERSTATE_ACCEPTING:
			retval = 0;
			break;

		case PARALLELTCP_SERVERSTATE_CLOSED:
			retval = -1;
			break;

		case PARALLELTCP_SERVERSTATE_ERROR:
			retval = -1;
			break;

		default:
			// Invalid State
			SON_PRINT_ERROR(" PTCP SERVER not INITIALIZED.\n  \
					You need to call init(port) or specify port \
					in the constructor %d :  %s \n ", __LINE__, __FILE__);
			retval = -1;
			break;
	}

	return retval;
}


/* performance Monitoring is enabled by default */
SON_paralleltcpClient :: SON_paralleltcpClient () : 
				m_numSockets(SON_PROTOCOL::SON_PARALLELTCP_NUMSOCKETS),
				m_maxSockFd(-1),
				m_sendBufSize(SON_PROTOCOL::SON_SENDBUFFER_SIZE_DEFAULT),
				m_recvBufSize(SON_PROTOCOL::SON_RECVBUFFER_SIZE_DEFAULT),
				m_state(PARALLELTCP_CLIENTSTATE_UNDEF),
				m_perfStatus(1)
{

	m_bigB = SON_BigBrother::getObj();

	m_protectState =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectState, NULL);
	
	/* Create the Perf Object */
	m_perfDB =  new SON_perf;
}


SON_paralleltcpClient :: SON_paralleltcpClient (int numsocks) : 
				m_numSockets(numsocks),
				m_maxSockFd(-1),
				m_sendBufSize(SON_PROTOCOL::SON_SENDBUFFER_SIZE_DEFAULT),
				m_recvBufSize(SON_PROTOCOL::SON_RECVBUFFER_SIZE_DEFAULT),
				m_state(PARALLELTCP_CLIENTSTATE_UNDEF),
				m_perfStatus(1)
{

	m_bigB = SON_BigBrother::getObj();

	m_protectState =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectState, NULL);
	
	/* Create the Perf Object */
	m_perfDB =  new SON_perf;
}

SON_paralleltcpClient::~SON_paralleltcpClient()
{
	/* CHECK if state is CLOSED; If not call close first */
	if (PARALLELTCP_CLIENTSTATE_CLOSED != this->getState())
		this->close();
	
	delete m_perfDB;
	m_perfDB = 0;
													
	pthread_rwlock_destroy (m_protectState);
	delete m_protectState;
	m_protectState = 0;
}



int SON_paralleltcpClient :: close()
{
	if (PARALLELTCP_CLIENTSTATE_CLOSED == this->getState())
		return 0;
	
	this->setState(PARALLELTCP_CLIENTSTATE_CLOSED);

	int closeResult = 0;
	
	for (unsigned int i = 0; i < m_clientList.size(); i++)
	{
		m_clientList[i]->close();
		delete m_clientList[i];
		m_clientList[i] = 0; 
	}
	m_clientList.clear();
	
	m_sockFd.clear();	
																			
	return closeResult;
}


int  SON_paralleltcpClient :: connectToServer(const char* ip, int port)
{
	char portstr[SON_STRING_SIZE_SMALL];
	sprintf(portstr, "%d", port);

	return this->connectToServer(ip, portstr);
}


int  SON_paralleltcpClient :: connectToServer(const char* ip, char* service)
{
    
	SON_PRINT_DEBUG3(" SON_paralleltcpClient : Connecting to SERVER %s and port %s \n", \
					ip, service);

	int retryAttempts = SON_CONN_RETRIES, retval = -1;

	while( retryAttempts > 0)
	{
		retval = this->__connectToServer(ip, service);	
		
		if (-1 == retval)
		{
			retryAttempts--;
			usleep(SON_CONN_RETRY_DELAY_USECS_LONG);
		}
		else
			break;
	}
	
    return retval;
}

void SON_paralleltcpClient :: setTotalSockets(int numsocks)
{
	m_numSockets = numsocks;
	return;
}
		
int SON_paralleltcpClient :: getTotalSockets(void)
{
	return m_numSockets;
}


int  SON_paralleltcpClient :: __connectToServer(const char* ip, char* service)
{
	int retval = -1;
	SON_tcpClient* client  =  new SON_tcpClient;
	
	retval = client->connectToServer(ip, service);
	
	if (-1 == retval)
		return retval;

	int numsocks = m_numSockets;
	long long int numsocks_size = sizeof(int);
	
	retval = client->write( (char*)&numsocks, numsocks_size);
	if (-1 == retval)
		return retval;
	
	char dataport[SON_STRING_SIZE_SMALL];
	memset(dataport, '\0', SON_STRING_SIZE_SMALL * sizeof(char));
	long long int dataport_size = SON_STRING_SIZE_SMALL;
	
	retval = client->read(dataport, dataport_size);	
	if (-1 == retval)		
		return retval;

	SON_PRINT_DEBUG3( \
		" SON_paralleltcpClient : DataPort is  %s : int: %d \n", 
		dataport, atoi(dataport));

	SON_tcpClient* dataclient;		
	int connectedclients = 0;
	while ( connectedclients < this->m_numSockets)
	{
		dataclient = new SON_tcpClient;
		retval = dataclient->connectToServer(ip, dataport);
		
		if (-1 == retval)
		{
			delete dataclient;
			dataclient = 0;
			
			/* Delete all the contents of the vectors */
			vector<SON_tcpClient*> :: iterator client_itr;			
			for (client_itr = m_clientList.begin();
				client_itr != m_clientList.end() ; client_itr++)
			{
				(*client_itr)->close();
				m_clientList.erase(client_itr);
			}
			m_clientList.clear();
			
			m_sockFd.clear();
			
			return -1;
		}
		else
		{
			this->m_clientList.push_back(dataclient);
			this->m_sockFd.push_back(dataclient->getSockFd());
			connectedclients++;
		}
	}			

	this->__setMaxSockFd();
	this->setState(PARALLELTCP_CLIENTSTATE_CONNECTED);
	this->__setNonBlocking();

	SON_PRINT_DEBUG3( \
		" SON_paralleltcpClient : Connected to SERVER %s and port %s \n", 
		ip,service);

    return 0;
}


void SON_paralleltcpClient :: setSendBufSize (int size)
{
	m_sendBufSize = size;
	
	if (PARALLELTCP_CLIENTSTATE_CONNECTED == this->getState())
		this->__setSendBufSize();
}



void SON_paralleltcpClient :: __setSendBufSize (void)
{
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		(*client_itr)->setSendBufSize(m_sendBufSize);
	}				
	return;
}


int SON_paralleltcpClient :: getSendBufSize (void)
{
	if (PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState())
		return this->m_sendBufSize;
		
	/* Loop through all the clients and get the min value */
	int minsize = INT_MAX, bufsize ;
	
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		bufsize = (*client_itr)->getSendBufSize();
		if (minsize < bufsize)
			minsize = bufsize;
	}
	return minsize;	
}


void SON_paralleltcpClient :: setRecvBufSize (int size)
{
	m_recvBufSize = size;
	
	if (PARALLELTCP_CLIENTSTATE_CONNECTED == this->getState())
		this->__setRecvBufSize();
}


void SON_paralleltcpClient :: __setRecvBufSize (void)
{
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		(*client_itr)->setRecvBufSize(m_recvBufSize);
	}				
	return;
}



int SON_paralleltcpClient :: getRecvBufSize (void)
{
	if (PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState())
		return this->m_recvBufSize;
		
	/* Loop through all the clients and get the min value */
	int minsize = INT_MAX, bufsize ;
	
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		bufsize = (*client_itr)->getRecvBufSize();
		if (minsize < bufsize)
			minsize = bufsize;
	}
	return minsize;	
}



SON_PARALLELTCP_CLIENTSTATE SON_paralleltcpClient :: getState(void)
{
	SON_PARALLELTCP_CLIENTSTATE retval;
	
	pthread_rwlock_rdlock(m_protectState);
	retval = m_state;
	pthread_rwlock_unlock(m_protectState);
	
	return retval;
}


void SON_paralleltcpClient :: setState(SON_PARALLELTCP_CLIENTSTATE state)
{	
	pthread_rwlock_wrlock(m_protectState);
	m_state = state;
	pthread_rwlock_unlock(m_protectState);
	
	return ;
}


int SON_paralleltcpClient :: getRemoteIP(char* ip)
{
	return m_clientList[0]->getRemoteIP(ip);
}


int SON_paralleltcpClient::getRemotePort(void)
{
	return m_clientList[0]->getRemotePort();
}

int SON_paralleltcpClient::getSelfIP(char* ip)
{
	return m_clientList[0]->getSelfIP(ip);
}


int SON_paralleltcpClient::getSelfPort(void)
{
	return m_clientList[0]->getSelfPort();
}


int SON_paralleltcpClient :: __setBlocking(void)
{
	int retval = 0;
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		retval = (*client_itr)->__setBlocking();
	}
	
	return retval;
}

int SON_paralleltcpClient :: __setNonBlocking(void)
{
	int retval = 0;
	vector<SON_tcpClient*> :: iterator client_itr;			
	for (client_itr = m_clientList.begin();
		client_itr != m_clientList.end() ; client_itr++)
	{
		retval = (*client_itr)->__setNonBlocking();
	}
	
	return retval;
}


void SON_paralleltcpClient :: __setMaxSockFd(void)
{
	
	vector<int> :: iterator sockfd_itr;			
	for (sockfd_itr = m_sockFd.begin();
		sockfd_itr != m_sockFd.end() ; sockfd_itr++)
	{
		if ( m_maxSockFd < *sockfd_itr)
			m_maxSockFd = *sockfd_itr;
	}
}



int SON_paralleltcpClient :: read(char *ptr, long long int& nbytes)
{
	if ( (0 == ptr) || (nbytes < 0) || 
		(PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_paralleltcpClient::Read: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return -1;
	}

	int numsocks = 0, maxsockfd = 0;
	char** bufptr;
	long long int* buflen;
	
	/* If we are sending less than 64K, use just 1 socket */
	if ( SON_PARALLELTCP_MINSIZE > nbytes)
	{
		numsocks = 1;
		maxsockfd = m_sockFd[0];
	}
	else
	{
		numsocks = this->m_numSockets;
		maxsockfd = this->m_maxSockFd;
	}
		
	bufptr =  new char* [numsocks];
	buflen = new long long int [numsocks];
		
	/* One can evaluate the effects of block partitioning
	* There is merit in making this partitioning Cache efficient
	* instead of the current static partitioning
	*/
	for (int i = 0; i <  numsocks; i++)
	{
		bufptr[i] = &ptr[ i * nbytes/numsocks];
		buflen[i] = nbytes / numsocks;
	}
	/* Take care of the last entry */
	buflen[numsocks - 1] += nbytes % numsocks;

	fd_set read_set;
	long long int nleft, nread, totread = 0;
	int myerr, select_fd;
	nleft = nbytes;

	while (nleft > 0)
	{
		FD_ZERO(&read_set);
		/* set the sockets on which we have to still send data */
		for (int i = 0; i <  numsocks; i++)
		{
			if ( buflen[i] > 0)
				FD_SET(m_sockFd[i], &read_set);
		}
		
    	select_fd = select( maxsockfd + 1, &read_set, NULL,  NULL, NULL);

				
		/* now loop through all sock FD and work on the ones that are set */
		
		if ( 0 == select_fd )
		{
			perror(":");
			SON_PRINT_PERROR( \
			" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
			__LINE__,__FILE__);

			this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
			nbytes =  totread;
			return -1;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR: 
					continue;
					break;
				
				default:
					perror(":");
					SON_PRINT_PERROR( \
					" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return -1;
					break;
			}
		}
		
		for ( int i = 0; i < numsocks ; i++) 
		{
			if ( buflen[i] <= 0)
				continue;
			
			if (FD_ISSET(m_sockFd[i], &read_set))
			{
				/* Write the Data */
				//nwrite = ::write(m_sockFd,ptr,nleft);
				nread = ::read(m_sockFd[i],bufptr[i],buflen[i]);
				if (nread > 0)
				{
					nleft -= nread;
					buflen[i] -= nread;
					bufptr[i] += nread;
					totread += nread;
				}		
				else 
				{
					if (0 == nread)
						continue;
			
					myerr = errno;
					switch (myerr)
					{
						#ifdef __APPLE__
						case EWOULDBLOCK:
						#endif
						#ifdef __linux__
						case EAGAIN:
						#endif
						case EINTR:			

						default:
							SON_PRINT_PERROR( \
							" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
							__LINE__,__FILE__);

							this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
							nbytes =  nread;
							return -1;
							break;
					} 
				}// Else
			}
		} // For
	} // while nleft > 0
		
		
	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}
	
	return 0;
}



int SON_paralleltcpClient :: write(const char *ptr, long long int& nbytes)
{
	if ( (0 == ptr) || (nbytes < 0) || 
		(PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_paralleltcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return -1;
	}

	int numsocks = 0, maxsockfd = 0;
	char** bufptr;
	long long int* buflen;
	
	/* If we are sending less than 64K, use just 1 socket */
	if ( SON_PARALLELTCP_MINSIZE > nbytes)
	{
		numsocks = 1;
		maxsockfd = m_sockFd[0];
	}
	else
	{
		numsocks = this->m_numSockets;
		maxsockfd = this->m_maxSockFd;
	}
		
	bufptr =  new char* [numsocks];
	buflen = new long long int [numsocks];
		
	/* One can evaluate the effects of block partitioning
	* There is merit in making this partitioning Cache efficient
	* instead of the current static partitioning
	*/
	for (int i = 0; i <  numsocks; i++)
	{
		bufptr[i] = (char*)&ptr[ i * nbytes/numsocks];
		buflen[i] = nbytes / numsocks;
	}
	/* Take care of the last entry */
	buflen[numsocks - 1] += nbytes % numsocks;

	fd_set write_set;
	long long int nleft, nwrite, totwrite = 0;
	int myerr, select_fd;
	nleft = nbytes;

	while (nleft > 0)
	{	
		FD_ZERO(&write_set);
		/* set the sockets on which we have to still send data */
		for (int i = 0; i <  numsocks; i++)
		{
			if ( buflen[i] > 0)
				FD_SET(m_sockFd[i], &write_set);
		}
		
    	select_fd = select( maxsockfd + 1,NULL, &write_set,  NULL, NULL);
				
		/* now loop through all sock FD and work on the ones that are set */
		
		if ( 0 == select_fd )
		{
			perror(":");
			SON_PRINT_PERROR( \
			" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
			__LINE__,__FILE__);

			this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
			nbytes =  totwrite;
			return -1;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR: 
					continue;
					break;
				
				default:
					perror(":");
					SON_PRINT_PERROR( \
					" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
					nbytes =  totwrite;
					return -1;
					break;
			}
		}	
			
		for ( int i = 0; i < numsocks ; i++) 
		{
			if ( buflen[i] <= 0)
				continue;
			
			if (FD_ISSET(m_sockFd[i], &write_set))
			{
				/* Write the Data */
				//nwrite = ::write(m_sockFd,ptr,nleft);
				nwrite = ::write(m_sockFd[i],bufptr[i],buflen[i]);
				if (nwrite > 0)
				{
					nleft -= nwrite;
					buflen[i] -= nwrite;
					bufptr[i] += nwrite;
					totwrite += nwrite;
				}		
				else 
				{
					if (0 == nwrite)
						continue;
			
					myerr = errno;
					switch (myerr)
					{
						#ifdef __APPLE__
						case EWOULDBLOCK:
						#endif
						#ifdef __linux__
						case EAGAIN:
						#endif
						case EINTR:			

						default:
							SON_PRINT_PERROR( \
							" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
							__LINE__,__FILE__);

							this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
							nbytes =  totwrite;
							return -1;
							break;
					} 
				}// Else
			}
		} // For
	} // while nleft > 0

	nbytes =  totwrite;
	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}
	
	return 0;
}


void SON_paralleltcpClient :: enablePerfMonitoring(void)
{
	m_perfStatus = 1;
}
		
void SON_paralleltcpClient :: disablePerfMonitoring(void)
{
	m_perfStatus = 0;
}

int SON_paralleltcpClient :: isMonitoringEnabled(void)
{
	return m_perfStatus;
}


double SON_paralleltcpClient :: getReadBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getReadBW();
	else 
		return (double)-1;
}



double SON_paralleltcpClient :: getWriteBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getWriteBW();
	else
		return (double)-1;	
}


double SON_paralleltcpClient :: getTotalBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getTotalBW();
	else
		return (double)-1;	
}


double SON_paralleltcpClient :: getDataRead(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getDataRead();
	else
		return (double)-1;
}


double SON_paralleltcpClient :: getDataWritten(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getDataWritten();
	else
		return (double)-1;
}


struct timeval SON_paralleltcpClient :: getConnStartTime(void)
{
	return m_perfDB->getConnStartTime();
}

void SON_paralleltcpClient :: __updateDataRead (double val)
{
	return m_perfDB->updateDataRead(val);
}

void SON_paralleltcpClient :: __updateDataWritten (double val)
{
	return m_perfDB->updateDataWritten(val);
}



int SON_paralleltcpClient :: readv(SON_iovec *recv_iovec, long long int& nbytes)
{
	if ( (0 == recv_iovec) || (nbytes < 0) || 
		(PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_paralleltcpClient::Read: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return -1;
	}

	int numsocks = 0, maxsockfd = 0;
	//char** bufptr;
	//long long int* buflen;
	
	int ioveccount  = recv_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return -1;
	}

	SON_PRINT_DEBUG3( \
		" SON_paralleltcpClient::readv -- Used Buffers =%d \n", ioveccount);

	/*
	*	Compute the Number of Sockets to use
	*
	*	If we are sending less than 64K, use just 1 socket
	*/
	if (( SON_PARALLELTCP_MINSIZE > nbytes) || (ioveccount == 1))
	{
		/* Use only a single connection */
		numsocks = 1;
		maxsockfd = m_sockFd[0];
	}
	else
	{
		/* We need more than 1 */
		
		//numsocks =  ioveccount / SON_PROTOCOL::SON_PARALLELTCP_MINBUFFERS;
		if ( ioveccount / this->m_numSockets) 
		{
			numsocks = this->m_numSockets;
		}
		else
			numsocks = ioveccount % this->m_numSockets;
		
		maxsockfd = m_sockFd[numsocks -1];
	}
	
	
	/*	Preapare IOVEC for each of the receiving sockets
	*	and Initialize the structures for them to use
	*/
	struct iovec** tempIovec = new struct iovec* [numsocks];
	int *tempIovcount =  new int[numsocks];
	void **savebase = new void* [numsocks];
	long long int* savelen =  new long long int[numsocks];
	
	
	for (int i = 0; i <  numsocks; i++)
	{	
		if ( i < (ioveccount % numsocks))
			tempIovec[i] = recv_iovec->getIovecPtr( 
							(i * (ioveccount / numsocks)) + i); 
		else 
			tempIovec[i] = recv_iovec->getIovecPtr(i * (ioveccount / numsocks));
							
		tempIovcount[i] = ioveccount / numsocks;
		if ( i < (ioveccount % numsocks))
			tempIovcount[i]++;
		
		savelen[i] =  tempIovec[i]->iov_len;
		savebase[i] = tempIovec[i]->iov_base;
	}
	
	
	/* Now Read the Data */
	
	fd_set read_set;
	long long int nleft, nread, totread = 0;
	int myerr, select_fd;
	nleft = nbytes;
	
	int countleft = ioveccount;

	while (countleft > 0)
	{
		FD_ZERO(&read_set);
	
		/* set the sockets on which we have to still Recv data */
		for (int i = 0; i <  numsocks; i++)
		{
			if (tempIovcount[i] > 0)
				FD_SET(m_sockFd[i], &read_set);
		}
		
    	select_fd = select( maxsockfd + 1, &read_set, NULL,  NULL, NULL);
				
		/* now loop through all sock FD and work on the ones that are set */
		if ( 0 == select_fd )
		{
			SON_PRINT_PERROR( \
			" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
			__LINE__,__FILE__);

			this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
			nbytes =  totread;
			return -1;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR: 
					continue;
					break;
				
				default:
					SON_PRINT_PERROR( \
					" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return -1;
					break;
			}
		}
		
		/* select was a sucess... Lets read some data */
		for ( int i = 0; i < numsocks ; i++) 
		{
			if ( tempIovcount[i] <= 0)
				continue;
			
			if (!FD_ISSET(m_sockFd[i], &read_set))
				continue;
			
			nread = ::readv(m_sockFd[i], tempIovec[i], tempIovcount[i]);
			
			if (nread < 0)
			{
				myerr = errno;
				switch(myerr)
				{				
					/* System call interrupted... We will try again */
					#ifdef __APPLE__
					case EWOULDBLOCK:
					#endif
					#ifdef __linux__
					case EAGAIN:
					#endif
					case EINTR:
						nread = 0;
						continue;
						break;
					
					/* Encountered an error. Retrun an error */
					default:
						SON_PRINT_PERROR( \
							" SON_paralleltcpClient : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec[i]->iov_len = savelen[i];
						tempIovec[i]->iov_base = savebase[i];
						nbytes =  totread;
						this->setState(PARALLELTCP_CLIENTSTATE_ERROR);

						return -1;
						break;
				}
			} //ENDOF if (nread < 0)
			
			else if (nread == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec[i]->iov_len= savelen[i];
				tempIovec[i]->iov_base= savebase[i];
				
				while ((ioveccount > 0) && (tempIovec[i]->iov_len == 0))
				{
					tempIovec[i]++;
					tempIovcount[i]--;
					countleft--;
				}
				if ( tempIovcount[i] == 0)
					break;
					
				if ( tempIovcount[i] > 0)
				{
					nbytes =  totread;
					SON_PRINT_INFO( \
					" SON_paralleltcpClient : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
				  	return -1;
					break;
				}
			} // ENDOF else if (nread == 0)
			
			else if (nread > 0)
			{
				/* Normal Condition */
				totread += nread;

				while (nread > 0)
				{
					if (nread >= (long long int)tempIovec[i]->iov_len)
					 {
						nread -= tempIovec[i]->iov_len;
		  				tempIovec[i]->iov_len = savelen[i];
						tempIovec[i]->iov_base = (char*) savebase[i];
						tempIovec[i]++;
						//curindex++;
						tempIovcount[i]--;
						countleft--;

	 					if ( tempIovcount[i] > 0)
						{
							savelen[i] = tempIovec[i]->iov_len;
							savebase[i] = tempIovec[i]->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec[i]->iov_len -= nread;
			  			tempIovec[i]->iov_base = (char *)tempIovec[i]->iov_base + nread;
	  					nread =	0;

						if ((tempIovcount[i] > 0) && (tempIovec[i]->iov_len == 0))
						{
							tempIovec[i]->iov_len = savelen[i];
							tempIovec[i]->iov_base = (char*) savebase[i];
							tempIovec[i]++;
							//curindex++;
							tempIovcount[i]--;
							countleft--;
							savelen[i] = tempIovec[i]->iov_len;
							savebase[i] = tempIovec[i]->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // ENDOF if (nread > 0)
			
		} // End of FOR i = 0; i < numsocks ; i++
	
	} // ENDOF While (countleft > 0)
	
	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}
	
	return 0;
}



int SON_paralleltcpClient :: writev(SON_iovec *send_iovec, long long int& nbytes)
{
	if ( (0 == send_iovec) || (nbytes < 0) || 
		(PARALLELTCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_paralleltcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return -1;
	}

	int numsocks = 0, maxsockfd = 0;
	//char** bufptr;
	//long long int* buflen;
	
	int ioveccount  = send_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return -1;
	}

	SON_PRINT_DEBUG3( \
		" SON_paralleltcpClient::writev -- Used Buffers =%d \n", ioveccount);

	/*
	*	Compute the Number of Sockets to use
	*
	*	If we are sending less than 64K, use just 1 socket
	*/
	if (( SON_PARALLELTCP_MINSIZE > nbytes) ||
			(ioveccount == 1))
	{
		/* Use only a single connection */
		numsocks = 1;
		maxsockfd = m_sockFd[0];
	}
	else
	{
		/* We need more than 1 */
		
		//numsocks =  ioveccount / SON_PROTOCOL::SON_PARALLELTCP_MINBUFFERS;
		if (( ioveccount / this->m_numSockets) > 1)
		{
			numsocks = this->m_numSockets;
		}
		else
			numsocks = ioveccount % this->m_numSockets;
		
		maxsockfd = m_sockFd[numsocks -1];
	}
	
	
	/*	Preapare IOVEC for each of the receiving sockets
	*	and Initialize the structures for them to use
	*/
	struct iovec** tempIovec = new struct iovec* [numsocks];
	int *tempIovcount =  new int[numsocks];
	void **savebase = new void* [numsocks];
	long long int* savelen =  new long long int[numsocks];
	
	
	for (int i = 0; i <  numsocks; i++)
	{	
		if ( i < (ioveccount % numsocks))
			tempIovec[i] = send_iovec->getIovecPtr( 
							(i * (ioveccount / numsocks)) + i); 
		else 
			tempIovec[i] = send_iovec->getIovecPtr(i * (ioveccount / numsocks));
							
		tempIovcount[i] = ioveccount / numsocks;
		if ( i < (ioveccount % numsocks))
			tempIovcount[i]++;
		
		savelen[i] =  tempIovec[i]->iov_len;
		savebase[i] = tempIovec[i]->iov_base;
	}
	
	fd_set write_set;
	long long int nleft, nwrite, totwrite = 0;
	int myerr, select_fd;
	nleft = nbytes;
	
	int countleft = ioveccount;

	while (countleft > 0)
	{
		FD_ZERO(&write_set);
	
		/* set the sockets on which we have to still Recv data */
		for (int i = 0; i <  numsocks; i++)
		{
			if (tempIovcount[i] > 0)
				FD_SET(m_sockFd[i], &write_set);
		}
		
    	select_fd = select( maxsockfd + 1, NULL, &write_set,  NULL, NULL);
				
		/* now loop through all sock FD and work on the ones that are set */
		if ( 0 == select_fd )
		{
			SON_PRINT_PERROR( \
			" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
			__LINE__,__FILE__);

			this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
			nbytes =  totwrite;
			return -1;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR: 
					continue;
					break;
				
				default:
					SON_PRINT_PERROR( \
					" SON_paralleltcpClient : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
					nbytes =  totwrite;
					return -1;
					break;
			}
		}
		
		/* select was a sucess... Lets read some data */
		for ( int i = 0; i < numsocks ; i++) 
		{
			if ( tempIovcount[i] <= 0)
				continue;
			
			if (!FD_ISSET(m_sockFd[i], &write_set))
				continue;
			
			nwrite = ::writev(m_sockFd[i], tempIovec[i], tempIovcount[i]);
			
			if (nwrite < 0)
			{
				myerr = errno;
				switch(myerr)
				{				
					/* System call interrupted... We will try again */
					#ifdef __APPLE__
					case EWOULDBLOCK:
					#endif
					#ifdef __linux__
					case EAGAIN:
					#endif
					case EINTR:
						nwrite = 0;
						continue;
						break;
					
					/* Encountered an error. Retrun an error */
					default:
						SON_PRINT_PERROR( \
							" SON_paralleltcpClient : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec[i]->iov_len = savelen[i];
						tempIovec[i]->iov_base = savebase[i];
						nbytes =  totwrite;
						this->setState(PARALLELTCP_CLIENTSTATE_ERROR);

						return -1;
						break;
				}
			} //ENDOF if (nread < 0)
			
			else if (nwrite == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec[i]->iov_len= savelen[i];
				tempIovec[i]->iov_base= savebase[i];
				
				while ((ioveccount > 0) && (tempIovec[i]->iov_len == 0))
				{
					tempIovec[i]++;
					tempIovcount[i]--;
					countleft--;
				}
				if ( tempIovcount[i] == 0)
					break;
					
				if ( tempIovcount[i] > 0)
				{
					nbytes =  totwrite;
					SON_PRINT_INFO( \
					" SON_paralleltcpClient : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(PARALLELTCP_CLIENTSTATE_ERROR);
				  	return -1;
					break;
				}
			} // ENDOF else if (nread == 0)
			
			else if (nwrite > 0)
			{
				/* Normal Condition */
				totwrite += nwrite;

				while (nwrite > 0)
				{
					if (nwrite >= (long long int)tempIovec[i]->iov_len)
					 {
						nwrite -= tempIovec[i]->iov_len;
		  				tempIovec[i]->iov_len = savelen[i];
						tempIovec[i]->iov_base = (char*) savebase[i];
						tempIovec[i]++;
						//curindex++;
						tempIovcount[i]--;
						countleft--;

	 					if ( tempIovcount[i] > 0)
						{
							savelen[i] = tempIovec[i]->iov_len;
							savebase[i] = tempIovec[i]->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec[i]->iov_len -= nwrite;
			  			tempIovec[i]->iov_base = (char *)tempIovec[i]->iov_base + nwrite;
	  					nwrite = 0;

						if ((tempIovcount[i] > 0) && (tempIovec[i]->iov_len == 0))
						{
							tempIovec[i]->iov_len = savelen[i];
							tempIovec[i]->iov_base = (char*) savebase[i];
							tempIovec[i]++;
							//curindex++;
							tempIovcount[i]--;
							countleft--;
							savelen[i] = tempIovec[i]->iov_len;
							savebase[i] = tempIovec[i]->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // ENDOF if (nwrite > 0)
			
		} // End of FOR i = 0; i < numsocks ; i++
	
	} // ENDOF While (countleft > 0)
	
	nbytes =  totwrite;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}
	
	return 0;
}


}


