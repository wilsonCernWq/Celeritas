#ifndef __SON__CELERITAS__CLIENT__H
#define __SON__CELERITAS__CLIENT__H

#ifndef __SON__CMATH
#define __SON__CMATH
#include <cmath>
#endif

#ifndef __SON__ERRNO
#define __SON__ERRNO
#include <cerrno>
#endif

#ifndef __SON__PERF__H
#include "SON_perf.h"
#endif


#ifndef __SON__TCP__H
#include "SON_tcp.h"
#endif

#ifndef __SON__TIMER__H
#include "SON_timer.h"
#endif


#ifndef __SON__BITSET
#define __SON__BITSET
#include <bitset>
#endif

/*! \namespace SON */
namespace SON_PROTOCOL
{

const int CELERITAS_HDR_ELEM = 2;
// MSG ID and PACKET ID (or Loss Packets)

class SON_celeritasServer;

class SON_celeritasClient
{
	public:

		SON_celeritasClient ();

		virtual ~SON_celeritasClient ();

		int connectToServer (const char *ip, int port);

        int connectToServer (const char *ip, char* service);

        int close (void);

        void printConnectionInfo ();

        int read(char *ptr, long long& nbytes);

        //int readZeroCopy(char *ptr, long long& nbytes);

        int write (const char *ptr, long long& nbytes);

        int setRate ( int rate_in_mbps);

        int setPayloadSize (int payloadsize);

		int setSendBufSize (int size);

		int getSendBufSize (void);

		int setRecvBufSize (int size);

		int getRecvBufSize (void);

		double getReadBW(void);

		double getWriteBW(void);

		double getTotalBW(void);

		double getDataRead(void);

		double getDataWritten(void);

		struct timeval getConnStartTime(void);

		void initPerf(void);

        /*
        int readv(SON_iovec *recv_iovec, long long& nbytes);

        int writev(SON_iovec *send_iovec, long long& nbytes);

		*/

	public:

		friend class SON_celeritasServer;

	protected:

		void __setTCPClient (SON_tcpClient* client);

		int __initLocalUDPPort (void);

		int __readRemoteUDPPort (void);

		int __sendLocalUDPPort (void);

		int __createUDPSocket (void);

		int __connectUDPSocket (void);

		int __getLocalUDPPort (void);

		void __usecDelay ( int& usec);

		void __msecDelay ( int& msec);

		int __writeData ( 	const char *ptr,
							const long long& buf_size,
							long long& bytes_sent,
							unsigned int& msgid,
							unsigned int& payloadsize,
							unsigned int& firstpacketid,
							unsigned int& numpackets);

		int __setUDPNonBlocking (void);

		void __incMsgID (void);

		int  __retransmitErrorBitList (	const char* ptr,
										const long long& msg_size,
										long long& totwrite,
										unsigned int& msgid,
										vector < bitset <32> >& error_list,
										unsigned int& error_list_size,
										unsigned int* loss_array,
										unsigned int& loss_array_size,
										unsigned int& lost_packets);

		int  __sendErrorList (	unsigned int& msgid,
								vector < bitset <32> >& error_list,
								unsigned int& error_list_size,
								unsigned int* loss_array,
								unsigned int& loss_array_size,
								unsigned int& lost_packets);

		int __recvLostData (	char* ptr,
								long long& bytes_read,
								unsigned int& msgid,
								vector < bitset <32> >& error_list,
								unsigned int& error_list_size,
								unsigned int* loss_array,
								unsigned int& loss_array_size,
								unsigned int& lost_packets);

		int __bufferedRead (	char* ptr,
								long long& nbytes,
								unsigned int& msgid,
								unsigned int& first_packet,
								unsigned int& totpackets,
								vector < bitset <32> >& error_list,
								unsigned int& error_list_size,
								unsigned int& lost_packets);


		int  __zeroCopyRead ( 	char* ptr,
								long long& nbytes,
								unsigned int& msgid,
								unsigned int& first_packet,
								unsigned int& totpackets,
								vector < bitset <32> >& error_list,
								unsigned int& error_list_size,
								unsigned int& lost_packets);

	protected:

		SON_tcpClient* m_tcpClient;

		int m_tcpSockFd;

		int m_udpSockFd;

		int m_localUDPPort;

		int m_remoteUDPPort;

		int m_payloadSize;

		int m_payloadDataSize;

		int m_sleepIntervalUsec;

		SON_perf m_perfDB;

		unsigned int m_msgID;

};

} // End of Namespace

#endif
