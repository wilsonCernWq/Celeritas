/******************************************************************************
 * Copyright (C) 2006 Venkatram Vishwanath,  Electronic Visualization Laboratory,  
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

#ifndef __SON__PARALLEL__TCP__H
#define __SON__PARALLEL__TCP__H

#ifndef __SON__TCP__H
#include "SON_tcp.h"
#endif

#include <vector>

namespace SON_PROTOCOL
{


/*! \enum SON_PARALLELTCP_SERVERSTATE Parallel TCP Server State Machine 
*	\brief This enumerated represents the State Transition Diagram of the
*	Parallel TCP Server
*/
enum SON_PARALLELTCP_SERVERSTATE
{
	PARALLELTCP_SERVERSTATE_UNDEF,
		/*!< Server State is Undefined */
	PARALLELTCP_SERVERSTATE_CREATED,
		/*!< Server has been Created */
	PARALLELTCP_SERVERSTATE_INITIALIZED,
		/*!< Server has been Initialized */
	PARALLELTCP_SERVERSTATE_ACCEPTING,
		/*!< Server is ready to Accept Connections */
	PARALLELTCP_SERVERSTATE_ERROR,
		/*!< Server is in an ERROR State */
	PARALLELTCP_SERVERSTATE_CLOSED
		/*!< Server has closed and will not accept any connections */
};


/* Forward Declaration */
class SON_paralleltcpClient;


class SON_paralleltcpServer
{
	public:

		SON_paralleltcpServer ();

		SON_paralleltcpServer (int senbufsize, int recvbufsize);

		virtual ~SON_paralleltcpServer();

		// void setAddr(char* addr);

		int init(int connport);
		
		int init(int connport, int dataport);

		int init(char* connport);
		
		int init(char* connport, char* dataport);
		
		int close (void);
		
		SON_paralleltcpClient* waitForNewConnection(void);
		
		virtual SON_paralleltcpClient* checkForNewConnection(int timeoutinsecs);
		
		virtual void setSendBufSize (int size);
		
		virtual int getSendBufSize (void);
		
		virtual void setRecvBufSize (int size);
		
		virtual int getRecvBufSize (void);
		

	protected:

		SON_BigBrother* m_bigB;
		
		SON_tcpServer* m_connServer;
		
		SON_tcpServer* m_dataServer;
		
		SON_PARALLELTCP_SERVERSTATE m_state;
		
		int m_sendBufSize;
		
		int m_recvBufSize;
		
		
	private:
		
		int __isInitialized(void);
};



/*! \enum SON_PARALLELTCP_CLIENTSTATE Parallel TCP Client State Machine 
*	\brief This enumerated represents the State Transition Diagram of the
*	Parallel TCP Server
*/
enum SON_PARALLELTCP_CLIENTSTATE
{
	PARALLELTCP_CLIENTSTATE_UNDEF,
		/*!< State is Undefined */
	PARALLELTCP_CLIENTSTATE_CONNECTED,
		/*!< Client is in a Connected State */
	PARALLELTCP_CLIENTSTATE_ERROR,
		/*!< Client is in an Error State */
	PARALLELTCP_CLIENTSTATE_CLOSED
		/*!< Client has Closed its Connection */
};



class SON_paralleltcpClient
{   
	public:

		SON_paralleltcpClient();
		
		SON_paralleltcpClient(int numsocks);
		
		virtual ~SON_paralleltcpClient();
		
		int connectToServer(const char *ip, int port);

		int connectToServer(const char *ip, char* service);
		
		void setTotalSockets(int numsocks);
		
		int getTotalSockets(void);
		
		int close(void);
		
		int write(const char *ptr, long long int& nbytes);
		
		int read( char *ptr, long long int& nbytes);

		int readv(SON_iovec *recv_iovec, long long& nbytes);
		
		int  writev(SON_iovec *send_iovec, long long& nbytes);
		
		SON_PARALLELTCP_CLIENTSTATE getState(void);
		
		void setState(SON_PARALLELTCP_CLIENTSTATE state);
		
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
		
		struct timeval getConnStartTime(void);

	public:
	
		friend class SON_paralleltcpServer;
		
	protected:

		int m_numSockets;
		
		int m_maxSockFd;
		
		int m_sendBufSize;
		
		int m_recvBufSize;
		
		vector<int> m_sockFd;
		
		SON_IO_MODE m_mode;
		
		vector<SON_tcpClient*> m_clientList;
				
		pthread_rwlock_t* m_protectState;

		SON_PARALLELTCP_CLIENTSTATE m_state;
		
		int m_perfStatus;

		SON_perf* m_perfDB;
	
		SON_BigBrother* m_bigB;
	
	protected:	
		
		int __connectToServer(const char *ip, char* service);
		
		int __setBlocking(void);
		
		int __setNonBlocking(void);
		
		void __setMaxSockFd(void);
		
		void __updateDataRead (double val);
		
		void __updateDataWritten (double val);
		
		void __setSendBufSize (void);
		
		void __setRecvBufSize (void);
		
};

			

}
#endif
