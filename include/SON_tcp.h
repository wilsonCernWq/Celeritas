/******************************************************************************
 * Copyright (C) 2006-2008 
 * Venkatram Vishwanath, 
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

#ifndef __SON__TCP__H
#define __SON__TCP__H

#ifndef __SON__IOVEC__H
#include "SON_iovec.h"
#endif

#ifndef __SON__BB__H
#include "SON_bigbrother.h"
#endif


#ifndef __SON__PERF__H
#include "SON_perf.h"
#endif


#ifndef __SON__COMMON__H
#include "SON_common.h"
#endif

/*! \namespace SON_TCP Celeritas TCP namespace */
namespace SON_PROTOCOL
{

/*! \enum SON_TCP_SERVERSTATE TCP Server State Machine 
*	\brief This enumerated represents the State Transition Diagram of the
*	TCP Server
*/
enum SON_TCP_SERVERSTATE
{
	TCP_SERVERSTATE_UNDEF,
		/*!< Server State is Undefined */
	TCP_SERVERSTATE_CREATED,
		/*!< Server has been Created */
	TCP_SERVERSTATE_INITIALIZED,
		/*!< Server has been Initialized */
	TCP_SERVERSTATE_ACCEPTING,
		/*!< Server is ready to Accept Connections */
	TCP_SERVERSTATE_ERROR,
		/*!< Server is in an ERROR State */
	TCP_SERVERSTATE_CLOSED
		/*!< Server has closed and will not accept any connections */
};


/* Forward Declaration */
class SON_tcpClient;


class SON_tcpServer 
{		
	public:

		SON_tcpServer();
		
		SON_tcpServer(int sendbufsize, int recvbufsize);

		virtual ~SON_tcpServer();

		virtual void setAddr(char* addr);
		
		virtual int init(int port);

		virtual int init(char* port);

		virtual SON_tcpClient* waitForNewConnection();

		virtual SON_tcpClient* checkForNewConnection(int timeoutinsecs);

		virtual int close (void);
		
		virtual void setSendBufSize (int size);
		
		virtual int getSendBufSize (void);
		
		virtual void setRecvBufSize (int size);
		
		virtual int getRecvBufSize (void);
		
		virtual char* getPort(void);

	protected:

		SON_BigBrother* m_bigB;

		char* m_serverAddr;
		
		char* m_serverPort;

		int m_serverFd;

		SON_TCP_SERVERSTATE m_serverState;
		
		int m_sendBufSize;
		
		int m_recvBufSize;
		
	protected:
		
		virtual int __initialize(void);
		
		virtual void __handleSelectError( int& flag, int& retry_attempts);
		
		virtual void __handleAcceptError( int& flag);
		
		virtual void __setLinger(int timeinsecs);
		
		virtual void __reuseAddr(void);
		
		virtual void __setNoDelay(void);
		
		virtual void __setSendBufSize (void);
		
		virtual void __setRecvBufSize (void);
};



/*! \enum SON_TCP_CLIENTSTATE  TCP Server State Machine 
*	\brief This enumerated represents the State Transition Diagram of the
*	 TCP Server
*/
enum SON_TCP_CLIENTSTATE
{
	TCP_CLIENTSTATE_UNDEF,
		/*!< State is Undefined */
	TCP_CLIENTSTATE_CONNECTED,
		/*!< Client is in a Connected State */
	TCP_CLIENTSTATE_ERROR,
		/*!< Client is in an Error State */
	TCP_CLIENTSTATE_CLOSED
		/*!< Client has Closed its Connection */
};




class SON_tcpClient
{
	public:

		SON_tcpClient();
		
		SON_tcpClient( int sendbufsize, 
						int recvbufsize, int perfenabled = 1) ;

		virtual ~SON_tcpClient();
		
		int connectToServer(const char *ip, int port);

		int connectToServer(const char *ip, char* service);
		
		int close(void);

		int read(char *ptr, long long& nbytes);
		
		int read(char *ptr, long long& nbytes, int timeout_sec);

		int readv(SON_iovec *recv_iovec, long long& nbytes);
		
		int write(const char *ptr, long long& nbytes);
		
		int writev(SON_iovec *send_iovec, long long& nbytes);
		
		SON_TCP_CLIENTSTATE getState(void);
		
		void setState(SON_TCP_CLIENTSTATE state);
		
		void setSendBufSize (int size);
		
		int getSendBufSize (void);
		
		void setRecvBufSize (int size);
		
		int getRecvBufSize (void);
		
		int getSelfIP(char* ip);

		int getRemoteIP(char* ip);

		int getSelfPort();

		int getRemotePort();
		
		void enablePerfMonitoring (void);
		
		void disablePerfMonitoring (void);
		
		int isMonitoringEnabled (void);
		
		double getReadBW(void);
		
		double getWriteBW(void);
		
		double getTotalBW(void);
		
		double getDataRead(void);
		
		double getDataWritten(void);
	
		int initTimer(void);
	
		struct timeval getConnStartTime(void);
		
		int getSockFd();
		
		void setIOMode(SON_IO_MODE mode);
		
		SON_IO_MODE getIOMode(void);
		
		int __setBlocking(void);
		
		int __setNonBlocking(void);
		
		
	public:
	
		friend class SON_tcpServer;
		
	protected:

		int m_sockFd;
		
		SON_IO_MODE m_mode;
		
		int m_blockingFlags;
		
		int m_selfAddrLen;

		struct sockaddr_storage m_selfAddr;
		
		int m_sendBufSize;
		
		int m_recvBufSize;
		
		pthread_rwlock_t* m_protectState;

		SON_TCP_CLIENTSTATE m_clientState;

		int m_perfStatus;

		SON_perf* m_perfDB;
		
		SON_BigBrother* m_bigB;
		
		
	protected:	
		
		int __connectToServer(const char *ip, char* service);
		
		void __setSockFd (int fd);
		
		void __updateDataRead (double val);
		
		void __updateDataWritten (double val);
		
		void __setLinger(int timeinsecs);
		
		void __reuseAddr(void);
		
		void __setNoDelay(void);
		
		void __setSendBufSize (void);
		
		void __setRecvBufSize (void);
		
		int __initializeSockFlags(void);
		
	private:
		
		int __blockingRead(char *ptr, long long int& nbytes);
		
		int __blockingReadV(SON_iovec *recv_iovec, long long int& nbytes);
		
		int __blockingWrite( const char *ptr, long long int& nbytes);
		
		int __blockingWriteV(SON_iovec *send_iovec, long long int& nbytes);
		
		int __nonblockingRead(char *ptr, long long int& nbytes);
		
		int __nonblockingReadT (char *ptr,
								long long int& nbytes,
								int timeout_sec);

		int __nonblockingReadV(SON_iovec *recv_iovec, long long int& nbytes);

		int  __nonblockingWrite(const char *ptr, long long int& nbytes);
		
		int __nonblockingWriteV(SON_iovec *send_iovec, long long int& nbytes);
				
};


}
#endif


