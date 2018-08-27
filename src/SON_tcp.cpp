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

#include "SON_tcp.h"
#include <stdlib.h>

namespace SON_PROTOCOL
{

extern  SON_BigBrother m_bigbrother;


SON_tcpServer :: SON_tcpServer():
					m_serverAddr(0),
					m_serverFd(-1),
					m_serverState(TCP_SERVERSTATE_UNDEF),
					m_sendBufSize(SON_PROTOCOL::SON_SENDBUFFER_SIZE_DEFAULT),
					m_recvBufSize(SON_PROTOCOL::SON_RECVBUFFER_SIZE_DEFAULT)
{
	m_bigB = SON_BigBrother::getObj();

	m_serverPort = new char[SON_STRING_SIZE_SMALL];
	memset(m_serverPort, '\0', SON_STRING_SIZE_SMALL * sizeof(char));

	m_serverState = TCP_SERVERSTATE_CREATED;
}


SON_tcpServer :: SON_tcpServer(int sendbufsize, int recvbufsize):
					m_serverAddr(0),
					m_serverFd(-1),
					m_serverState(TCP_SERVERSTATE_UNDEF),
					m_sendBufSize(sendbufsize),
					m_recvBufSize(recvbufsize)
{
	m_bigB = SON_BigBrother::getObj();

	m_serverPort = new char[SON_STRING_SIZE_SMALL];
	memset(m_serverPort, '\0', SON_STRING_SIZE_SMALL * sizeof(char));

	m_serverState = TCP_SERVERSTATE_CREATED;
}


SON_tcpServer :: ~SON_tcpServer()
{
	/* CHECK if state is CLOSED; If not call close first */
	if (m_serverState != TCP_SERVERSTATE_CLOSED)
		this->close();

	/* Delete Dynamically allocated Port String */
	if (m_serverPort)
	{
		delete [] m_serverPort;
		m_serverPort = 0;
	}
}

void SON_tcpServer :: setAddr(char* addr)
{
	if ( 0 == m_serverAddr)
	{
		m_serverAddr = new char[SON_STRING_SIZE_SMALL];
		memset(m_serverAddr, '\0', SON_STRING_SIZE_SMALL * sizeof(char));
		strncpy(m_serverAddr, addr, strlen(addr));
	}
	else
		strncpy(m_serverAddr, addr, strlen(addr));
	
	return;
}


int SON_tcpServer::init(int port)
{
	char myport[SON_STRING_SIZE_SMALL];
	sprintf(myport, "%d", port);
	
	int retval = this->init(myport);
	
	return retval;
}

int SON_tcpServer::init (char* port)
{
	if (m_serverState == TCP_SERVERSTATE_INITIALIZED)
		return SON_PROTOCOL::SUCCESS;

	SON_PRINT_DEBUG2(" SON_tcpServer::init For Port %s \n", port);

	strncpy(m_serverPort, port, strlen(port));

	struct addrinfo hints, *res, *ressave;
    int error;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen  = 0;
	hints.ai_addr     = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next     = NULL;

    error = ::getaddrinfo(m_serverAddr, m_serverPort, &hints, &res);
    if (error != 0)
	{
        fprintf(stderr,
                "getaddrinfo error:: [%s]\n",
                gai_strerror(error));
        return SON_PROTOCOL::FAILURE;
    }

    ressave = res;

    /* ************************************************************ */
    /*   Try open socket with each address getaddrinfo returned,	*/
    /*   until getting a valid listening socket.					*/
    /* ************************************************************ */

	m_serverFd = -1;

	while(res)
	{
        m_serverFd = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol);

		if (m_serverFd < 0)
			continue;

        if (m_serverFd >= 0)
		{
			/* Set the Linger and Reuse Address Option */
			this->__reuseAddr();
			this->__setLinger(1000);
			
			/* Disable Nagle */
			this->__setNoDelay();
			
			/* Set the Send and Recv Buffer Sizes */
			this->__setSendBufSize();
			this->__setRecvBufSize();

            if (::bind(m_serverFd, res->ai_addr, res->ai_addrlen) == 0)
                break;
				
			perror ( " BIND : ");
            ::close(m_serverFd);
            m_serverFd = -1;
        }

		res = res->ai_next;
    }

	if ((m_serverFd < 0)|| (res == NULL))
	{
		SON_PRINT_ERROR(" Opening Socket :: %d :  %s \n ", \
			__LINE__, __FILE__);
		m_serverState = TCP_SERVERSTATE_ERROR;
        return SON_PROTOCOL::FAILURE;
    }


	// In case of TCP
	int myret =-1;
	myret = ::listen(m_serverFd, SON_LISTEN_QUEUE_SIZE);
	if (-1 == myret )
	{
		SON_PRINT_ERROR(" Listen :: %d :  %s \n ", __LINE__, __FILE__);
		::close(m_serverFd);
		m_serverFd =-1;
		m_serverState = TCP_SERVERSTATE_ERROR;
		return SON_PROTOCOL::FAILURE;
	}

	// Make the socket Nonblocking
	int val =  fcntl(m_serverFd , F_GETFL, 0);
	if (val < 0)
		perror(" fcntl : " );

	if (fcntl(m_serverFd , F_SETFL, val | O_NONBLOCK) == -1)
		perror(" FCNTL :  ");

	m_serverState = TCP_SERVERSTATE_INITIALIZED;
	::freeaddrinfo(ressave);

    return SON_PROTOCOL::SUCCESS;
}


SON_tcpClient* SON_tcpServer::waitForNewConnection()
{
	if (-1 == this->__initialize())
		return 0;
	
	fd_set read_set;
	int ready_fd, newsockfd, continueflag = 1, num_retries = 0;
	
	while(continueflag)
	{
		// Perform a non-blocking check to see if socket is ready for
		// an incoming connection.
		FD_ZERO(&read_set);
		FD_SET(m_serverFd, &read_set);

    	ready_fd = select( m_serverFd+1, &read_set, NULL, NULL, NULL);

		/*  If The server socket FD is NOT the one set..
		*	Retry if we have not exceeded the number of retries
		*/	
		if ( ( 0 == ready_fd) || (!FD_ISSET( m_serverFd, &read_set) ) )
		{
			num_retries++;
			if (num_retries >= SON_CONN_RETRIES)
			{
				continueflag = 0;
				return 0;
			}
			else
				continue;
		}

		/* Check if select encountered an Error and handle it */
		if ( ready_fd < 0)
		{
			this->__handleSelectError(continueflag, num_retries);
			if (0 == continueflag)
				return 0;
			else
				continue;
		}

		/* Select was a a success and the descriptor is ready
		* Now we try and Accept the Connection
		*/
		SON_tcpClient* clientObj = new SON_tcpClient;

		clientObj->m_selfAddrLen =  sizeof(sockaddr_storage);
		memset (&(clientObj->m_selfAddr), '\0' , clientObj->m_selfAddrLen );
		
		newsockfd = ::accept(m_serverFd, \
						(struct sockaddr *)&(clientObj->m_selfAddr),  \
						(socklen_t *) &clientObj->m_selfAddrLen);

		if (newsockfd > 0)
		{
			clientObj->__setSockFd(newsockfd);
			clientObj->setState(TCP_CLIENTSTATE_CONNECTED);
		
			/* Set reuse Addr and Linger */
			clientObj->__reuseAddr();
			clientObj->__setLinger(1000);
			
			/* Disable Nagle */
			clientObj->__setNoDelay();
			
			/* Set the Send and Recv Buffer Sizes */
			clientObj->__setSendBufSize();
			clientObj->__setRecvBufSize();
			
			/* Set the Socket flags */
			clientObj->__initializeSockFlags();
			
			continueflag = 0;
		    return clientObj;
		}
		else
		{
			this->__handleAcceptError(continueflag);
			delete clientObj;
			clientObj = 0;
		}
	}

	return 0;
}


/*
Need to handle non blcoking conditions

*/
SON_tcpClient* SON_tcpServer::checkForNewConnection(int timeoutinsecs)
{	
	if (-1 == this->__initialize())
		return 0;

	int ready_fd, newsockfd;
	
	// Perform a non-blocking check to see if socket is ready for
	// an incoming connection.
	fd_set read_set;
	struct timeval timeout;
	timeout.tv_sec = timeoutinsecs;
	timeout.tv_usec = 0;
	
	FD_ZERO(&read_set);
	FD_SET(m_serverFd, &read_set);
	
    ready_fd = select( m_serverFd+1, &read_set, NULL, NULL, &timeout);

    /* Time out has occured,or FD of interest is not SET */
    if ((0 == ready_fd)  ||  (!FD_ISSET( m_serverFd, &read_set)))
		return 0;

	/* Check if select encountered an Error and handle it */
	if ( ready_fd < 0)
	{
		/* The below variables are not used in this case and need for 
		* the function's arguments 
		*/
		int continueflag = 0, num_retries = 0;
		
		this->__handleSelectError(continueflag, num_retries);
		return 0;
	}
	
	/* Select was a a success and the descriptor is ready
	* Now we try and Accept the Connection
	*/
	SON_tcpClient *clientObj = new SON_tcpClient;

	clientObj->m_selfAddrLen =  sizeof(sockaddr_storage);
	memset (&(clientObj->m_selfAddr), '\0' , clientObj->m_selfAddrLen );
	newsockfd = ::accept(this->m_serverFd, \
				(struct sockaddr *)&(clientObj->m_selfAddr), \
				(socklen_t *) &clientObj->m_selfAddrLen);

	if (newsockfd > 0)
	 {
		clientObj->__setSockFd(newsockfd);
		clientObj->setState(TCP_CLIENTSTATE_CONNECTED);
	
		/* Set reuse Addr and Linger */
		clientObj->__reuseAddr();
		clientObj->__setLinger(1000);
			
		/* Disable Nagle */
		clientObj->__setNoDelay();
			
		/* Set the Send and Recv Buffer Sizes */
		clientObj->__setSendBufSize();
		clientObj->__setRecvBufSize();
		
		/* Set the Socket flags */
		clientObj->__initializeSockFlags();

	    return clientObj;
	}
	else
	{
		/* The below variables are not used in this case and need 
		* for the function's arguments
		*/
		int continueflag = 0;
	
		this->__handleAcceptError(continueflag);
		delete clientObj;
		clientObj = 0;
	}

	return 0;
}


int SON_tcpServer :: close()
{
	m_serverState = TCP_SERVERSTATE_CLOSED;

	/* Close the Server Port */
	int closeResult = 0;
	if (m_serverFd >= 0)
	{
		closeResult = ::close(m_serverFd);
		m_serverFd = 0;
	}

	if (0 == closeResult)
		return SON_PROTOCOL::SUCCESS;		
	else
		return SON_PROTOCOL::FAILURE;
}



void SON_tcpServer :: setRecvBufSize (int size)
{
	m_recvBufSize = size;
	
	if (TCP_SERVERSTATE_INITIALIZED == m_serverState)
		this->__setRecvBufSize();
}


int SON_tcpServer :: getRecvBufSize (void)
{
	if (TCP_SERVERSTATE_INITIALIZED != m_serverState)
		return this->m_recvBufSize;

	int retsize = -1, retsizelen;
	if (::getsockopt(this->m_serverFd,SOL_SOCKET,SO_RCVBUF, \
							(char *) &retsize,(socklen_t*) &retsizelen) < 0)
	{						
		SON_PRINT_ERROR(" GetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}	
	
	return retsize;
}




void SON_tcpServer :: setSendBufSize (int size)
{
	m_sendBufSize = size;
	
	if (TCP_SERVERSTATE_INITIALIZED == m_serverState)
		this->__setSendBufSize();
}


int SON_tcpServer :: getSendBufSize (void)
{
	if (TCP_SERVERSTATE_INITIALIZED != m_serverState)
		return this->m_sendBufSize;

	int retsize = -1, retsizelen;
	if (::getsockopt(this->m_serverFd,SOL_SOCKET,SO_SNDBUF, \
							(char *) &retsize,(socklen_t*) &retsizelen) < 0)
	{						
		SON_PRINT_ERROR(" GetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}	
	
	return retsize;
}


char* SON_tcpServer :: getPort (void)
{
	return m_serverPort;
}


int SON_tcpServer :: __initialize (void)
{
	int retval = -1;
	
	switch (this->m_serverState)
	{
		case  TCP_SERVERSTATE_INITIALIZED:
			this->m_serverState = TCP_SERVERSTATE_ACCEPTING;
			retval = 0;
			break;

		case  TCP_SERVERSTATE_ACCEPTING:
			retval = 0;
			break;

		case TCP_SERVERSTATE_CLOSED:
			retval = -1;
			break;

		case TCP_SERVERSTATE_ERROR:
			retval = -1;
			break;

		default:
			// Invalid State
			SON_PRINT_ERROR(" SERVER not INITIALIZED.\n  \
					You need to call init(port) or specify port \
					in the constructor %d :  %s \n ", __LINE__, __FILE__);
			retval = -1;
			break;
	}

	return retval;
}

void SON_tcpServer :: __handleSelectError (int& flag, int& retry_attempts)
{
	int syscall_error = errno;
	
	SON_PRINT_PERROR( " SERVER SELECT ERROR :: %d :  %s \n ", \
						__LINE__, __FILE__);
	switch (syscall_error)
	{
		/* Interrupted System Call.. Retry */
		case EINTR:
			retry_attempts++;					
			if (retry_attempts >= SON_CONN_RETRIES)
				flag = 0;
			break;
					
		#ifdef __APPLE__
		/* OSX Man Pages recommends to Try Again.. Retry */
		case EAGAIN:
			retry_attempts++;					
			if (retry_attempts >= SON_CONN_RETRIES)
				flag = 0;
			break;
		#endif
				
		/* Some error has occured */
		default:
			flag = 0;
			break;
	}
	
	return;
}


void SON_tcpServer :: __handleAcceptError( int& flag)
{
	SON_PRINT_PERROR( " SERVER ACCEPT ERROR :: %d :  %s \n ", \
						__LINE__, __FILE__);
	
	int syscall_error = errno;	
	switch(syscall_error)
	{
		case EINTR:
			flag = 1;
			break;

		default:
			flag = 0;
			break;
	}

	return;
}


void SON_tcpServer :: __setLinger (int timeinsecs = 1000)
{
	struct linger lingerData;
	lingerData.l_onoff = 1;
	lingerData.l_linger = timeinsecs;

	if (::setsockopt(this->m_serverFd,SOL_SOCKET,SO_LINGER,   \
						(char *) &lingerData,sizeof(lingerData)) < 0)
	{
		SON_PRINT_INFO(
		"SON_tcpServer ::Open: setsockopt: SO_LINGER failed.%d : %s \n \
		.. However We will Still proceed without setting it ...  \n",
		__LINE__, __FILE__);
	}

	return;	
}


void SON_tcpServer :: __reuseAddr (void)
{
	int reuse = 1;
	if (::setsockopt(this->m_serverFd,SOL_SOCKET,SO_REUSEADDR, &reuse, 
												sizeof(int)) == -1)
	{
		SON_PRINT_INFO( \
		"SON_tcpServer::  setsockopt: SO_REUSEADDR failed %d : %s \n \
		.. However We will Still proceed without setting it ...  \n", 
			__LINE__,__FILE__);
	}
	return;
}


void SON_tcpServer :: __setNoDelay (void)
{
	/* Set the NODELAY option */
	#ifdef __linux__
	int nodelay = 1;
	if (::setsockopt(this->m_serverFd,IPPROTO_TCP, TCP_NODELAY,&nodelay,
												sizeof(int)) == -1)
	{
		SON_PRINT_INFO( \
		" SON_tcpServer:: SetsockOpt : TCP_NODELAY Failed %d : %s \n  \
		... However We will Still proceed without setting it ...  \n",
				__LINE__, __FILE__);
	}
	#endif
	
	return;
}


void SON_tcpServer :: __setRecvBufSize (void)
{
	if (::setsockopt(this->m_serverFd,SOL_SOCKET,SO_RCVBUF, \
				(char *) &(this->m_recvBufSize), sizeof(int)) < 0)
		SON_PRINT_DEBUG1(" Error while trying to SetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
				
	return;
}


void SON_tcpServer :: __setSendBufSize (void)
{
	if (::setsockopt(this->m_serverFd,SOL_SOCKET,SO_SNDBUF, \
				(char *) &(this->m_sendBufSize), sizeof(int)) < 0)
		SON_PRINT_DEBUG1(" Error while trying to SetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
				
	return;
}



/* performance Monitoring is enabled by default */
SON_tcpClient :: SON_tcpClient() : 
				m_sockFd(-1),
				m_mode(SON_IO_BLOCKING),
				m_sendBufSize(SON_PROTOCOL::SON_SENDBUFFER_SIZE_DEFAULT),
				m_recvBufSize(SON_PROTOCOL::SON_RECVBUFFER_SIZE_DEFAULT),
				m_clientState(TCP_CLIENTSTATE_UNDEF),
				m_perfStatus(1)
{
	m_bigB = SON_BigBrother::getObj();
	
	m_protectState =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectState, NULL);
	
	/* Create the Perf Object */
	m_perfDB =  new SON_perf;	
}


SON_tcpClient :: SON_tcpClient(	int sendbufsize, 
								int recvbufsize,
								int perfenabled) : 
				m_sockFd(-1),
				m_mode(SON_IO_BLOCKING),
				m_sendBufSize(sendbufsize),
				m_recvBufSize(recvbufsize),
				m_clientState(TCP_CLIENTSTATE_UNDEF),
				m_perfStatus(perfenabled)
{
	m_bigB = SON_BigBrother::getObj();
	
	m_protectState =  new pthread_rwlock_t;
	pthread_rwlock_init(m_protectState, NULL);
	
	/* Create the Perf Object */
	m_perfDB =  new SON_perf;
}


SON_tcpClient::~SON_tcpClient()
{
	/* CHECK if state is CLOSED; If not call close first */
	if (TCP_CLIENTSTATE_CLOSED != this->getState())
		this->close();
	
	delete m_perfDB;
	m_perfDB = 0;
					
	pthread_rwlock_destroy (m_protectState);
	delete m_protectState;
	m_protectState = 0;
}



int SON_tcpClient::close()
{
	if (TCP_CLIENTSTATE_CLOSED == this->getState())
		return SON_PROTOCOL::SUCCESS;
	
	this->setState(TCP_CLIENTSTATE_CLOSED);

	int closeResult = 0;
	if (m_sockFd > 0)
	{
		::shutdown(m_sockFd,SHUT_RDWR);
		closeResult = ::close(m_sockFd);
		m_sockFd = 0;
	}
	
	return closeResult;
}


int  SON_tcpClient::connectToServer(const char* ip, int port)
{
	char portstr[SON_STRING_SIZE_SMALL];
	sprintf(portstr, "%d", port);

	return this->connectToServer(ip, portstr);
}


int  SON_tcpClient::connectToServer(const char* ip, char* service)
{
    
	SON_PRINT_DEBUG3(" TCPCLient : Connecting to SERVER %s and port %s \n", \
					ip, service);

	int retryAttempts = SON_CONN_RETRIES, retval = -1;

	while( retryAttempts > 0)
	{
		retval = this->__connectToServer(ip, service);	
		
		if (-1 == retval)
		{
			retryAttempts--;
			usleep(50000);
		}
		else
			break;
	}
	
	if ( 0 == retval)
		return SON_PROTOCOL::SUCCESS;

	return SON_PROTOCOL::FAILURE;
}



int  SON_tcpClient::__connectToServer(const char* ip, char* service)
{
    struct addrinfo hints, *res, *ressave;
    int error;
	
	memset(&hints, '\0', sizeof(struct addrinfo));
	//hints.ai_flags    = AI_PASSIVE;
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(ip, service, &hints, &res);
	if (error != 0)
	{
		fprintf(stderr,
			"getaddrinfo error:: [%s]\n",
			gai_strerror(error));
		
		return SON_PROTOCOL::FAILURE;
	}

	ressave = res;

	while (res)
	{
		this->m_sockFd = socket(res->ai_family,
							res->ai_socktype,
							res->ai_protocol);

		if (this->m_sockFd >= 0)
		{
			/* Set the Linger and Reuse Address Option */
			this->__reuseAddr() ;
			this->__setLinger(1000);
				
			/* Disable Nagle */
			this->__setNoDelay();
			
			/* Set the Send and Recv Buffer Sizes */
			this->__setSendBufSize();
			this->__setRecvBufSize();
			
			this->__initializeSockFlags();
			
			if (::connect(this->m_sockFd, res->ai_addr, res->ai_addrlen) == 0)
			{
				this->setState(TCP_CLIENTSTATE_CONNECTED);
				break;
			}	

			::close(this->m_sockFd);
			this->m_sockFd = -1;
			this->setState(TCP_CLIENTSTATE_ERROR);
		}
		
		res = res->ai_next;
	}


	if ((this->m_sockFd < 0) || (res == NULL) ||
		 ( TCP_CLIENTSTATE_ERROR == this->getState()) )
	{
		::freeaddrinfo(ressave);
		return SON_PROTOCOL::FAILURE;
	}
	
	this->setState(TCP_CLIENTSTATE_CONNECTED);

	SON_PRINT_DEBUG3( \
		" TCPCLient : Connected to SERVER %s and port %s \n", ip,service);

	::freeaddrinfo(ressave);

    return SON_PROTOCOL::SUCCESS;

}



int SON_tcpClient :: read(char *ptr, long long& nbytes, int timeout_sec)
{
	return this->__nonblockingReadT(ptr, nbytes, timeout_sec);
}


int SON_tcpClient :: read(char *ptr, long long& nbytes)
{
	switch(this->m_mode)
	{
		case SON_IO_BLOCKING:
			return this->__blockingRead(ptr, nbytes);
			break;
		
		case SON_IO_NONBLOCKING:
			return this->__nonblockingRead(ptr, nbytes);
			break;
		
		default:
			return this->__blockingRead(ptr, nbytes);
			break;
	}
}


int SON_tcpClient :: readv(SON_iovec *recv_iovec, long long& nbytes)
{
	switch(this->m_mode)
	{
		case SON_IO_BLOCKING:
			return this->__blockingReadV(recv_iovec, nbytes);
			break;
		
		case SON_IO_NONBLOCKING:
			return this->__nonblockingReadV(recv_iovec, nbytes);
			break;
		
		default:
			return this->__blockingReadV(recv_iovec, nbytes);
			break;
	}
}



int SON_tcpClient :: write(const char *ptr, long long& nbytes)
{
	switch(this->m_mode)
	{
		case SON_IO_BLOCKING:
			return this->__blockingWrite(ptr, nbytes);
			break;
			
		case SON_IO_NONBLOCKING:
			return this->__nonblockingWrite(ptr, nbytes);
			break;
			
		default:
			return this->__blockingWrite(ptr, nbytes);
			break;
	}
}


int SON_tcpClient :: writev(SON_iovec *send_iovec, long long& nbytes)
{
	switch(this->m_mode)
	{
		case SON_IO_BLOCKING:
			return this->__blockingWriteV(send_iovec, nbytes);
			break;
			
		case SON_IO_NONBLOCKING:
			return this->__nonblockingWriteV(send_iovec, nbytes);
			break;
			
		default:
			return this->__blockingWriteV(send_iovec, nbytes);
			break;
	}
}



int SON_tcpClient :: __blockingRead(char *ptr, long long int& nbytes)
{
	if ((m_sockFd < 0) || (ptr == 0) || (nbytes < 0) ||
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR(
		"SON_tcpClient::READ: INVALID Connection or Parameters .%d : %s \n ", \
		__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	long long int nleft, nread, totread =0;
	int myerr;
	nleft = nbytes;

	while (nleft > 0)
	{
		nread = ::read(m_sockFd,ptr,nleft);

		if (nread > 0)
		{
			nleft -= nread;
			ptr += nread;
			totread += nread;
		}
		else if (nread < 0)
		{
			myerr = errno;
			switch(myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR:
					nread =0;
					continue;
					break;

				default:
					SON_PRINT_PERROR( \
						" SON_TCPCLIENT : READ ERROR %d : %s \n ", \
						__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		else if (nread == 0)
		{
			if (nleft ==0)
				break;
			else
			{
				SON_PRINT_INFO( \
				" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

				this->setState(TCP_CLIENTSTATE_ERROR);
				nbytes =  totread;
				return SON_PROTOCOL::FAILURE;
				break;
			}
		}
	} // End of While 

	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}
	
	return SON_PROTOCOL::SUCCESS;
}


int SON_tcpClient :: __nonblockingReadT (char *ptr,
										long long int& nbytes,
										int timeout_sec)
{
	struct timeval tv;
	tv.tv_sec =  timeout_sec;
	tv.tv_usec = 0;

	if ((m_sockFd < 0) || (ptr == 0) || (nbytes < 0) ||
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR(
		"SON_tcpClient::READ: INVALID Connection or Parameters .%d : %s \n ", \
		__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	fd_set read_set;
	long long int nleft, nread, totread =0;
	int myerr, select_fd;
	nleft = nbytes;

	while (nleft > 0)
	{
		FD_ZERO(&read_set);
		FD_SET(m_sockFd, &read_set);

    	select_fd = select( m_sockFd+1, &read_set, NULL, NULL, &tv);
		
		if ( 0 == select_fd)
		{	/* Timer Has expired */
			nbytes =  totread;
			return SON_PROTOCOL::TIME_OUT;
		}
 
		if ( !FD_ISSET(m_sockFd, &read_set) )
		{
			perror(":");
			SON_PRINT_PERROR( \
				" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
				__LINE__,__FILE__);

			this->setState(TCP_CLIENTSTATE_ERROR);
			nbytes =  totread;
			return SON_PROTOCOL::FAILURE;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				case EINTR:
				#ifdef __APPLE__
				/* OSX Man Pages recommends to Try Again.. Retry */
				case EAGAIN:
				#endif
					continue;
					break;
				
				default:
					perror(":");
					SON_PRINT_PERROR( \
					" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		
		/* select was a sucess... Lets read some data */
		nread = ::read(m_sockFd,ptr,nleft);

		if (nread > 0)
		{
			nleft -= nread;
			ptr += nread;
			totread += nread;
		}
		else if (nread < 0)
		{
			myerr = errno;
			switch(myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR:
					nread =0;
					continue;
					break;

				default:
					SON_PRINT_PERROR( \
						" SON_TCPCLIENT : READ ERROR %d : %s \n ", \
						__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		else if (nread == 0)
		{
			if (nleft ==0)
				break;
			else
			{
				SON_PRINT_INFO( \
				" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

				this->setState(TCP_CLIENTSTATE_ERROR);
				nbytes =  totread;
				return SON_PROTOCOL::FAILURE;
				break;
			}
		}
	} // End of While 

	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}
	
	return SON_PROTOCOL::SUCCESS;
}


int SON_tcpClient :: __nonblockingRead(char *ptr, long long int& nbytes)
{
	if ((m_sockFd < 0) || (ptr == 0) || (nbytes < 0) ||
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR(
		"SON_tcpClient::READ: INVALID Connection or Parameters .%d : %s \n ", \
		__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	fd_set read_set;
	long long int nleft, nread, totread =0;
	int myerr, select_fd;
	nleft = nbytes;

	while (nleft > 0)
	{
		FD_ZERO(&read_set);
		FD_SET(m_sockFd, &read_set);

    	select_fd = select( m_sockFd+1, &read_set, NULL, NULL, NULL);
		
		if ((0 == select_fd) || !FD_ISSET(m_sockFd, &read_set) )
		{
			perror(":");
			SON_PRINT_PERROR( \
				" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
				__LINE__,__FILE__);

			this->setState(TCP_CLIENTSTATE_ERROR);
			nbytes =  totread;
			return SON_PROTOCOL::FAILURE;
		}
		
		if ( select_fd < 0)
		{
			myerr = errno;
			switch (myerr)
			{
				case EINTR:
				#ifdef __APPLE__
				/* OSX Man Pages recommends to Try Again.. Retry */
				case EAGAIN:
				#endif
					continue;
					break;
				
				default:
					perror(":");
					SON_PRINT_PERROR( \
					" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		
		/* select was a sucess... Lets read some data */
		nread = ::read(m_sockFd,ptr,nleft);

		if (nread > 0)
		{
			nleft -= nread;
			ptr += nread;
			totread += nread;
		}
		else if (nread < 0)
		{
			myerr = errno;
			switch(myerr)
			{
				#ifdef __APPLE__
				case EWOULDBLOCK:
				#endif
				#ifdef __linux__
				case EAGAIN:
				#endif
				case EINTR:
					nread =0;
					continue;
					break;

				default:
					SON_PRINT_PERROR( \
						" SON_TCPCLIENT : READ ERROR %d : %s \n ", \
						__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		else if (nread == 0)
		{
			if (nleft ==0)
				break;
			else
			{
				SON_PRINT_INFO( \
				" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

				this->setState(TCP_CLIENTSTATE_ERROR);
				nbytes =  totread;
				return SON_PROTOCOL::FAILURE;
				break;
			}
		}
	} // End of While 

	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}
	
	return SON_PROTOCOL::SUCCESS;
}



/*
int SON_tcpClient :: blockingRead( char *ptr, int& nbytes)
{
	long long int databytes  =  (long long int) nbytes;
	
	int retval = this->read( ptr, databytes); 
	
	nbytes = (int) databytes;
	
	return retval;
}
*/


int SON_tcpClient:: __blockingWrite(const char *ptr, long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == ptr) || (nbytes < 0) || 
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	long long int nleft, nwrite, totwrite = 0;
	int myerr;
	nleft = nbytes;

	while (nleft > 0)
	{		
		nwrite = ::write(m_sockFd,ptr,nleft);
		if (nwrite > 0)
		{
			nleft -= nwrite;
			ptr += nwrite;
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
					continue;
					break;				

				default:
					perror(":");
					SON_PRINT_PERROR( \
					" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
 					__LINE__,__FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totwrite;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
	}

	nbytes =  totwrite;
	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}
	
	return SON_PROTOCOL::SUCCESS;
}


/*
int SON_tcpClient :: blockingWrite(const char *ptr, int& nbytes)
{
	long long int databytes  =  (long long int) nbytes;
	
	int retval = this->write( ptr, databytes); 
	
	nbytes = (int) databytes;
	
	return retval;
}
*/



int SON_tcpClient:: __nonblockingWrite(const char *ptr, long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == ptr) || (nbytes < 0) || 
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	fd_set write_set;
	long long int nleft, nwrite, totwrite = 0;
	int myerr, select_fd;
	nleft = nbytes;

	while (nleft > 0)
	{	
		FD_ZERO(&write_set);
		FD_SET(m_sockFd, &write_set);

    	select_fd = select( m_sockFd+1,NULL, &write_set,  NULL, NULL);
		
		if ( (0 == select_fd) || (!FD_ISSET(m_sockFd, &write_set)) )
		{
			perror(":");
			SON_PRINT_PERROR( \
			" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
			__LINE__,__FILE__);

			this->setState(TCP_CLIENTSTATE_ERROR);
			nbytes =  totwrite;
			return SON_PROTOCOL::FAILURE;
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
					" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
					__LINE__,__FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totwrite;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
		
		/* select was a sucess... Lets write some data */
		nwrite = ::write(m_sockFd,ptr,nleft);
		if (nwrite > 0)
		{
			nleft -= nwrite;
			ptr += nwrite;
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
					perror(":");
					SON_PRINT_PERROR( \
					" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
 					__LINE__,__FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
					nbytes =  totwrite;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}
	}

	nbytes =  totwrite;
	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}
	
	return SON_PROTOCOL::SUCCESS;
}



SON_TCP_CLIENTSTATE SON_tcpClient :: getState(void)
{
	SON_TCP_CLIENTSTATE retval;
	
	pthread_rwlock_rdlock(m_protectState);
	retval = m_clientState;
	pthread_rwlock_unlock(m_protectState);
	
	return retval;
}


void SON_tcpClient :: setState(SON_TCP_CLIENTSTATE state)
{	
	pthread_rwlock_wrlock(m_protectState);
	m_clientState = state;
	pthread_rwlock_unlock(m_protectState);
	
	return ;
}


void SON_tcpClient :: setSendBufSize (int size)
{
	m_sendBufSize = size;
	
	if (TCP_CLIENTSTATE_CONNECTED == this->getState())
		this->__setSendBufSize();
}

void SON_tcpClient :: __setSendBufSize (void)
{
	if (::setsockopt(this->m_sockFd,SOL_SOCKET,SO_SNDBUF, \
					(char *)  &(this->m_sendBufSize), sizeof(int)) < 0)
		SON_PRINT_DEBUG1(" Error while trying to SetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
	return;
}


int SON_tcpClient :: getSendBufSize (void)
{
	if (TCP_CLIENTSTATE_CONNECTED != this->getState())
		return this->m_sendBufSize;
		
	int retsize = -1, retsizelen;
	if (::getsockopt(this->m_sockFd,SOL_SOCKET,SO_SNDBUF, \
							(char *) &retsize, (socklen_t*) &retsizelen) < 0)
	{						
		SON_PRINT_ERROR(" SetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}	
	
	return retsize;	
}


void SON_tcpClient :: setRecvBufSize (int size)
{
	m_recvBufSize = size;
	
	if (TCP_CLIENTSTATE_CONNECTED == this->getState())
		this->__setRecvBufSize();
}

void SON_tcpClient :: __setRecvBufSize (void)
{
	if (::setsockopt(this->m_sockFd,SOL_SOCKET,SO_RCVBUF, \
				(char *) &(this->m_recvBufSize), sizeof(int)) < 0)
		SON_PRINT_DEBUG1(" Error while trying to SetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
				
	return;
}



int SON_tcpClient :: getRecvBufSize (void)
{
	if (TCP_CLIENTSTATE_CONNECTED != this->getState())
		return this->m_recvBufSize;

	int retsize = -1, retsizelen;
	if (::getsockopt(this->m_sockFd,SOL_SOCKET,SO_RCVBUF, \
							(char *) &retsize,(socklen_t*) &retsizelen) < 0)
	{						
		SON_PRINT_ERROR(" GetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}	
	
	return retsize;
}


int SON_tcpClient :: getSockFd (void)
{
	return m_sockFd;
}


void SON_tcpClient :: __setSockFd( int fd)
{
	m_sockFd = fd;
}


int SON_tcpClient :: getRemoteIP(char* ip)
{
	int retval;
	struct sockaddr_storage addr;
	socklen_t addr_len =  sizeof(struct sockaddr_storage) ;

	retval = ::getpeername(m_sockFd,(struct sockaddr *) &addr, &addr_len);
	if (retval < 0)
	{
		perror( " getRemoteIP :: getpeername : " );
		return SON_PROTOCOL::FAILURE;
	}

	retval =  ::getnameinfo( (const struct sockaddr *) &addr, addr_len, ip,
												1024, 0,0, NI_NUMERICHOST);
	if (retval < 0)
	{
		perror( " getRemoteIP :: getnameinfo : " );
		return SON_PROTOCOL::FAILURE;
	}
	else
		return SON_PROTOCOL::SUCCESS;
}


int SON_tcpClient::getRemotePort(void)
{
	int retport;
	int retval;
	struct sockaddr_storage addr;
	socklen_t addr_len =  sizeof(struct sockaddr_storage) ;
	char myport[1024];

	retval = ::getpeername(m_sockFd,(struct sockaddr *) &addr, &addr_len);
	if (retval < 0)
	{
		perror( " getRemotePort :: getpeername : " );
		return SON_PROTOCOL::FAILURE;
	}

	retval =  ::getnameinfo( (const struct sockaddr *) &addr, addr_len,
									 0, 0, myport, 1024, NI_NUMERICSERV);
	if (retval < 0)
	{
		perror( "  getRemotePort ::getnameinfo : " );
		return SON_PROTOCOL::FAILURE;
	}
	else
	{
		retport = atoi(myport);
		return retport;
	}
}

int SON_tcpClient::getSelfIP(char* ip)
{
	int retval;
	struct sockaddr_storage addr;
	socklen_t addr_len =  sizeof(struct sockaddr_storage) ;

	retval = ::getsockname(m_sockFd,(struct sockaddr *) &addr, &addr_len);
	if (retval < 0)
	{
		perror( " getSelfIP :: getsockname :" );
		return SON_PROTOCOL::FAILURE;
	}
	retval =  ::getnameinfo( (const struct sockaddr *) &addr, addr_len, ip,
												 1024, 0,0, NI_NUMERICHOST);
	if (retval < 0)
	{
		perror( " getSelfIP :: getnameinfo : " );
		return SON_PROTOCOL::FAILURE;
	}
	else
		return SON_PROTOCOL::SUCCESS;
}


int SON_tcpClient::getSelfPort(void)
{
	int retport;
	int retval;
	struct sockaddr_storage addr;
	socklen_t addr_len =  sizeof(struct sockaddr_storage) ;
	char myport[1024];

	retval = ::getsockname(m_sockFd,(struct sockaddr *) &addr, &addr_len);
	if (retval < 0)
	{
		perror( " getRemotePort :: getpeername : " );
		return SON_PROTOCOL::FAILURE;
	}

	retval =  ::getnameinfo( (const struct sockaddr *) &addr, addr_len, 0, 0,
												myport, 1024, NI_NUMERICSERV);
	if (retval < 0)
	{
		perror( "  getRemotePort ::getnameinfo : " );
		return SON_PROTOCOL::FAILURE;
	}
	else
	{
		retport = atoi(myport);
		return retport;
	}

}


void SON_tcpClient :: enablePerfMonitoring(void)
{
	m_perfStatus = 1;
}
		
void SON_tcpClient :: disablePerfMonitoring(void)
{
	m_perfStatus = 0;
}

int SON_tcpClient :: isMonitoringEnabled(void)
{
	return m_perfStatus;
}


double SON_tcpClient :: getReadBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getReadBW();
	else 
		return (double)-1;
}



double SON_tcpClient :: getWriteBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getWriteBW();
	else
		return (double)-1;	
}


double SON_tcpClient :: getTotalBW(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getTotalBW();
	else
		return (double)-1;	
}


double SON_tcpClient :: getDataRead(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getDataRead();
	else
		return (double)-1;
}


double SON_tcpClient :: getDataWritten(void)
{
	if (likely(m_perfStatus))
		return m_perfDB->getDataWritten();
	else
		return (double)-1;
}


struct timeval SON_tcpClient :: getConnStartTime(void)
{
	return m_perfDB->getConnStartTime();
}


void SON_tcpClient :: setIOMode(SON_IO_MODE mode)
{
	m_mode =  mode;
	
	switch(this->m_mode)
	{
		case SON_IO_BLOCKING:
			this->__setBlocking();
			break;
			
		case SON_IO_NONBLOCKING:
			this->__setNonBlocking();
			break;
			
		default:
			this->__setBlocking();
			break;
	}
}

		
SON_IO_MODE SON_tcpClient :: getIOMode(void)
{
	return m_mode;
}

int SON_tcpClient::initTimer(void)
{
	m_perfDB->initConnStartTime();

	return SON_PROTOCOL::SUCCESS;
}


void SON_tcpClient :: __updateDataRead (double val)
{
	return m_perfDB->updateDataRead(val);
}

void SON_tcpClient :: __updateDataWritten (double val)
{
	return m_perfDB->updateDataWritten(val);
}


void SON_tcpClient :: __setLinger (int timeinsecs = 1000)
{
	struct linger lingerData;
	lingerData.l_onoff = 1;
	lingerData.l_linger = timeinsecs;

	if (::setsockopt(this->m_sockFd,SOL_SOCKET,SO_LINGER,   \
						(char *) &lingerData,sizeof(lingerData)) < 0)
	{
		SON_PRINT_ERROR(
		"SON_tcpClient::Open: setsockopt: SO_LINGER failed.%d : %s \n  \
		... However We will Still proceed without setting it ...  \n",
		__LINE__, __FILE__);
	}		
	return;	
}


void SON_tcpClient :: __reuseAddr (void)
{
	int reuse = 1;
	if (::setsockopt(this->m_sockFd,SOL_SOCKET,SO_REUSEADDR, &reuse, 
												sizeof(int)) == -1)
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::  setsockopt: SO_REUSEADDR failed %d : %s \n \
		... However We will Still proceed without setting it ...  \n",
			__LINE__,__FILE__);
	}
	
	return;	
}


void  SON_tcpClient :: __setNoDelay (void)
{
	/* Set the NODELAY option */
	//#ifdef __linux__
	int nodelay = 1;
	if (::setsockopt(this->m_sockFd,IPPROTO_TCP, TCP_NODELAY,&nodelay,
												sizeof(int)) == -1)
	{
		SON_PRINT_ERROR( \
		" SON_tcpClient:: SetsockOpt : TCP_NODELAY Failed %d : %s \n  \
		... However We will Still proceed without setting it ...  \n ", 
			__LINE__, __FILE__);
	}
	//#endif
	
	return;
}


int SON_tcpClient :: __initializeSockFlags(void)
{
	int retval = 0;
	
	this->m_blockingFlags =  ::fcntl(this->m_sockFd , F_GETFL, 0);
	if (this->m_blockingFlags < 0)
	{
		perror(" fcntl : " );
		retval =  -1;
	}
	
	return retval;
}


int SON_tcpClient :: __setBlocking(void)
{
	int retval = 0;

	if (::fcntl(this->m_sockFd , F_SETFL, this->m_blockingFlags) == -1)
	{
		perror(" FCNTL :  ");
		retval = -1;
	}
	
	return retval;
}

int SON_tcpClient :: __setNonBlocking(void)
{
	int retval = 0;
	
	if (::fcntl(this->m_sockFd, F_SETFL,
				this->m_blockingFlags | O_NONBLOCK) == -1)
	{
		perror(" FCNTL :  ");
		retval = -1;
	}
	
	return retval;
}

int SON_tcpClient:: __blockingReadV (SON_iovec *recv_iovec,
									long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == recv_iovec) ||  
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}


	int ioveccount  = recv_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	SON_PRINT_DEBUG3( \
		" SON_tcpClient::readv -- Used Buffers =%d \n", ioveccount);

	// I may have to check for the buffersizes too..
	long long int nread, savelen, totread = 0;

	struct iovec* tempIovec = NULL;
	void *savebase = NULL;
	int readcount  = 0;

	int countleft  = ioveccount;
	int curindex = 0, myerr;

	while(countleft > 0)
	{
		tempIovec = recv_iovec->getIovecPtr(curindex);
		savelen = tempIovec->iov_len;
		savebase = tempIovec->iov_base;

		readcount  = 0;
		if (countleft  >  (IOV_MAX-1))
			readcount  = (IOV_MAX-1);
		else
			readcount = countleft;

		while (readcount > 0 )
		{
			nread = ::readv(m_sockFd, tempIovec, readcount);
			
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
						nread =0;
						continue;
						break;
						
					default:
						SON_PRINT_PERROR( \
							" SON_TCPCLIENT : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec->iov_len = savelen;
						tempIovec->iov_base = savebase;
						nbytes =  totread;
						this->setState(TCP_CLIENTSTATE_ERROR);

						return SON_PROTOCOL::FAILURE;
						break;
				}
			}
			else if (nread == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec->iov_len= savelen;
				tempIovec->iov_base= savebase;
				
				while ((ioveccount > 0) && (tempIovec->iov_len == 0))
				{
					tempIovec++;
					curindex++;
					readcount--;
					countleft--;
				}

				if (readcount == 0)
					break;
					
				if (readcount != 0)
				{
					nbytes =  totread;
					SON_PRINT_INFO( \
					" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
				  	return SON_PROTOCOL::FAILURE;
					break;
				}
			}
			else if (nread > 0)
			{
				/* Normal Condition */
				totread += nread;

				while (nread > 0)
				{
					if (nread >= (long long int)tempIovec->iov_len)
					 {
						nread -= tempIovec->iov_len;
		  				tempIovec->iov_len = savelen;
						tempIovec->iov_base = (char*) savebase;
						tempIovec++;
						curindex++;
						readcount--;
						countleft--;

	 					if (readcount > 0)
						{
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec->iov_len -= nread;
			  			tempIovec->iov_base = (char *)tempIovec->iov_base 
												+ nread;
	  					nread =	0;

						if ((readcount > 0) && (tempIovec->iov_len == 0))
						{
							tempIovec->iov_len = savelen;
							tempIovec->iov_base = (char*) savebase;
							tempIovec++;
							curindex++;
							readcount--;
							countleft--;
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // end of IF IovecCOunt >0

		} // END OF WHILE

	} // end of FOR


	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}

	return SON_PROTOCOL::SUCCESS;
}



int SON_tcpClient:: __nonblockingReadV (SON_iovec *recv_iovec, 
										long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == recv_iovec) ||  
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}


	int ioveccount  = recv_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	SON_PRINT_DEBUG3( \
		" SON_tcpClient::readv -- Used Buffers =%d \n", ioveccount);

	long long int nread, savelen, totread = 0;
	fd_set read_set;
	struct iovec* tempIovec = NULL;
	void *savebase = NULL;
	int readcount  = 0;
	int countleft  = ioveccount;
	int curindex = 0, myerr, select_fd;

	while(countleft > 0)
	{
		tempIovec = recv_iovec->getIovecPtr(curindex);
		savelen = tempIovec->iov_len;
		savebase = tempIovec->iov_base;

		readcount  = 0;
		if (countleft  >  (IOV_MAX-1))
			readcount  = (IOV_MAX-1);
		else
			readcount = countleft;

		while (readcount > 0 )
		{
			FD_ZERO(&read_set);
			FD_SET(m_sockFd, &read_set);

			select_fd = select( m_sockFd+1, &read_set, NULL, NULL, NULL);
			
			if ( (0 == select_fd) || (!FD_ISSET(m_sockFd, &read_set)) )
			{
				tempIovec->iov_len = savelen;
				tempIovec->iov_base = savebase;
				SON_PRINT_PERROR( \
				" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
				__LINE__,__FILE__);

				this->setState(TCP_CLIENTSTATE_ERROR);
				nbytes =  totread;
				return SON_PROTOCOL::FAILURE;
			}

			/* select was a sucess... Lets read some data */
			nread = ::readv(m_sockFd, tempIovec, readcount);
			
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
						nread =0;
						continue;
						break;
						
					default:
						SON_PRINT_PERROR( \
							" SON_TCPCLIENT : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec->iov_len = savelen;
						tempIovec->iov_base = savebase;
						nbytes =  totread;
						this->setState(TCP_CLIENTSTATE_ERROR);

						return SON_PROTOCOL::FAILURE;
						break;
				}
			}
			else if (nread == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec->iov_len= savelen;
				tempIovec->iov_base= savebase;
				
				while ((ioveccount > 0) && (tempIovec->iov_len == 0))
				{
					tempIovec++;
					curindex++;
					readcount--;
					countleft--;
				}
				if (readcount == 0)
					break;
					
				if (readcount > 0)
				{
					nbytes =  totread;
					SON_PRINT_INFO( \
					" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
				  	return SON_PROTOCOL::FAILURE;
					break;
				}
			}
			else if (nread > 0)
			{
				/* Normal Condition */
				totread += nread;

				while (nread > 0)
				{
					if (nread >= (long long int)tempIovec->iov_len)
					 {
						nread -= tempIovec->iov_len;
		  				tempIovec->iov_len = savelen;
						tempIovec->iov_base = (char*) savebase;
						tempIovec++;
						curindex++;
						readcount--;
						countleft--;

	 					if (readcount > 0)
						{
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec->iov_len -= nread;
			  			tempIovec->iov_base = (char *)tempIovec->iov_base 
												+ nread;
	  					nread =	0;

						if ((readcount > 0) && (tempIovec->iov_len == 0))
						{
							tempIovec->iov_len = savelen;
							tempIovec->iov_base = (char*) savebase;
							tempIovec++;
							curindex++;
							readcount--;
							countleft--;
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // end of IF IovecCOunt >0

		} // END OF WHILE

	} // end of FOR


	nbytes =  totread;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataRead(val);
	}

	return SON_PROTOCOL::SUCCESS;
}





int SON_tcpClient :: __blockingWriteV (SON_iovec *send_iovec,
										long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == send_iovec) ||  
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}


	int ioveccount  = send_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	SON_PRINT_DEBUG3( \
		" SON_tcpClient::writev -- Used Buffers =%d \n", ioveccount);

	// I may have to check for the buffersizes too..
	long long int nwrite, savelen, totwrite = 0;

	struct iovec* tempIovec = NULL;
	void *savebase = NULL;
	int writecount  = 0;

	int countleft  = ioveccount;
	int curindex = 0, myerr;

	while(countleft > 0)
	{
		tempIovec = send_iovec->getIovecPtr(curindex);
		savelen = tempIovec->iov_len;
		savebase = tempIovec->iov_base;

		writecount  = 0;
		if (countleft  >  (IOV_MAX-1))
			writecount  = (IOV_MAX-1);
		else
			writecount = countleft;

		while (writecount > 0 )
		{
			nwrite = ::writev(m_sockFd, tempIovec, writecount);
			
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
						nwrite =0;
						continue;
						break;
						
					default:
						SON_PRINT_PERROR( \
							" SON_TCPCLIENT : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec->iov_len = savelen;
						tempIovec->iov_base = savebase;
						nbytes =  totwrite;
						this->setState(TCP_CLIENTSTATE_ERROR);

						return SON_PROTOCOL::FAILURE;
						break;
				}
			}
			else if (nwrite == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec->iov_len= savelen;
				tempIovec->iov_base= savebase;
				
				while ((ioveccount > 0) && (tempIovec->iov_len == 0))
				{
					tempIovec++;
					curindex++;
					writecount--;
					countleft--;
				}

				if (writecount == 0)
					break;
					
				if (writecount != 0)
				{
					nbytes =  totwrite;
					SON_PRINT_INFO( \
					" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
				  	return SON_PROTOCOL::FAILURE;
					break;
				}
			}
			else if (nwrite > 0)
			{
				/* Normal Condition */
				totwrite += nwrite;

				while (nwrite > 0)
				{
					if (nwrite >= (long long int)tempIovec->iov_len)
					 {
						nwrite -= tempIovec->iov_len;
		  				tempIovec->iov_len = savelen;
						tempIovec->iov_base = (char*) savebase;
						tempIovec++;
						curindex++;
						writecount--;
						countleft--;

	 					if (writecount > 0)
						{
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec->iov_len -= nwrite;
			  			tempIovec->iov_base = (char *)tempIovec->iov_base 
												+ nwrite;
	  					nwrite =	0;

						if ((writecount > 0) && (tempIovec->iov_len == 0))
						{
							tempIovec->iov_len = savelen;
							tempIovec->iov_base = (char*) savebase;
							tempIovec++;
							curindex++;
							writecount--;
							countleft--;
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // end of IF IovecCOunt >0

		} // END OF WHILE

	} // end of FOR


	nbytes =  totwrite;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}

	return SON_PROTOCOL::SUCCESS;
}



int SON_tcpClient :: __nonblockingWriteV (SON_iovec *send_iovec,
										 long long int& nbytes)
{
	if ((m_sockFd < 0) || (0 == send_iovec) ||  
		(TCP_CLIENTSTATE_CONNECTED != this->getState()) )
	{
		SON_PRINT_ERROR( \
		"SON_tcpClient::Write: INVALID Method Arguments %d : %s \n ", \
			__LINE__, __FILE__);
		
		this->setState(TCP_CLIENTSTATE_ERROR);
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}


	int ioveccount  = send_iovec->getUsedBuffers();
	if (ioveccount <= 0)
	{
		nbytes = 0;
		return SON_PROTOCOL::FAILURE;
	}

	SON_PRINT_DEBUG3( \
		" SON_tcpClient::readv -- Used Buffers =%d \n", ioveccount);

	// I may have to check for the buffersizes too..
	long long int nwrite, savelen, totwrite = 0;

	struct iovec* tempIovec = NULL;
	void *savebase = NULL;
	int writecount  = 0;

	int countleft  = ioveccount;
	int curindex = 0, myerr, select_fd;
	fd_set write_set;

	while(countleft > 0)
	{
		tempIovec = send_iovec->getIovecPtr(curindex);
		savelen = tempIovec->iov_len;
		savebase = tempIovec->iov_base;

		writecount  = 0;
		if (countleft  >  (IOV_MAX-1))
			writecount  = (IOV_MAX-1);
		else
			writecount = countleft;

		while (writecount > 0 )
		{
			FD_ZERO(&write_set);
			FD_SET(m_sockFd, &write_set);

			select_fd = select( m_sockFd+1,NULL, &write_set,  NULL, NULL);
		
			if ( (0 == select_fd) || (!FD_ISSET(m_sockFd, &write_set)) )
			{
			
				SON_PRINT_PERROR( \
				" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
				__LINE__,__FILE__);

				this->setState(TCP_CLIENTSTATE_ERROR);
				/* Reset the ptrs */
				tempIovec->iov_len = savelen;
				tempIovec->iov_base = savebase;
				nbytes =  totwrite;
				return SON_PROTOCOL::FAILURE;
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
						" SON_TCPCLIENT : WRITE ERROR %d : %s \n ", \
						__LINE__,__FILE__);

						this->setState(TCP_CLIENTSTATE_ERROR);
						/* Reset the ptrs */
						tempIovec->iov_len = savelen;
						tempIovec->iov_base = savebase;
						nbytes =  totwrite;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}
		
			nwrite = ::writev(m_sockFd, tempIovec, writecount);
			
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
						nwrite =0;
						continue;
						break;
						
					default:
						SON_PRINT_PERROR( \
							" SON_TCPCLIENT : READV ERROR %d : %s \n ", \
							__LINE__, __FILE__);
						
						/* Reset the ptrs */
						tempIovec->iov_len = savelen;
						tempIovec->iov_base = savebase;
						nbytes =  totwrite;
						this->setState(TCP_CLIENTSTATE_ERROR);

						return SON_PROTOCOL::FAILURE;
						break;
				}
			}
			else if (nwrite == 0)
			{	/* possible disconnection */
			
				/* Reset the ptrs */
				tempIovec->iov_len= savelen;
				tempIovec->iov_base= savebase;
				
				while ((ioveccount > 0) && (tempIovec->iov_len == 0))
				{
					tempIovec++;
					curindex++;
					writecount--;
					countleft--;
				}

				if (writecount == 0)
					break;
					
				if (writecount != 0)
				{
					nbytes =  totwrite;
					SON_PRINT_INFO( \
					" SON_TCPCLIENT : REMOTE CONNECTION CLOSED %d : %s \n ", \
 					__LINE__, __FILE__);

					this->setState(TCP_CLIENTSTATE_ERROR);
				  	return SON_PROTOCOL::FAILURE;
					break;
				}
			}
			else if (nwrite > 0)
			{
				/* Normal Condition */
				totwrite += nwrite;

				while (nwrite > 0)
				{
					if (nwrite >= (long long int)tempIovec->iov_len)
					 {
						nwrite -= tempIovec->iov_len;
		  				tempIovec->iov_len = savelen;
						tempIovec->iov_base = (char*) savebase;
						tempIovec++;
						curindex++;
						writecount--;
						countleft--;

	 					if (writecount > 0)
						{
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
					}
					else
					{
						/* in the same iovec structure */
						tempIovec->iov_len -= nwrite;
			  			tempIovec->iov_base = (char *)tempIovec->iov_base 
												+ nwrite;
	  					nwrite =	0;

						if ((writecount > 0) && (tempIovec->iov_len == 0))
						{
							tempIovec->iov_len = savelen;
							tempIovec->iov_base = (char*) savebase;
							tempIovec++;
							curindex++;
							writecount--;
							countleft--;
							savelen = tempIovec->iov_len;
							savebase = tempIovec->iov_base;
						}
      				}

    			}// end of While nREAD >0

			} // end of IF IovecCOunt >0

		} // END OF WHILE

	} // end of FOR


	nbytes =  totwrite;

	if (likely(m_perfStatus))
	{
		double val = (double) nbytes;
		this->__updateDataWritten(val);
	}

	return SON_PROTOCOL::SUCCESS;
}


} // End of namespace




