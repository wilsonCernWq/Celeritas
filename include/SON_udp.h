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

#ifndef __SON__UDP__H
#define __SON__UDP__H

#include "SON_common.h"
#include "SON_iovec.h"
#include <map>
#include <signal.h>

/** UDP Class. This class bypasses QUANTA's standard method for establishing connections and provides the user with direct control of UDP connections. The idea is that you first create a udp object and then call the init() method. (see init() method for more details).
Then you can either Send data to a destination or receive any incoming data.

 To send data to a destination you need to set the destination with the
 SetSendAddress method and then call the Send method. 

 To receive data, simply call the Receive method.

 By default the connections are BLOCKING, which means, if you call Receive and there is no data available to read your program will block until something comes along. To make your receives  or sends non blocking, use the NON_BLOCKING flag as a parameter to the receive() or send().In the previous versions the makeNonBlocking() method was used. The makeNonBlocking() is being phased out.
 

 * @version Version 5/26/97
 * @author (C) 1996 Jason Leigh
 */


class QUANTAnet_udp_c : public QUANTAnet_socketbase_c {

private:
	// Instrumentation flag.
	int enableInstr;

    // Socket file descriptor.
	int sockfd;

    // Socket structures.
	struct sockaddr_in selfAddress, sendAddress, receiveAddress;

   //For performance monitoring
    QUANTAnet_perfMonitor_c pmonitor;

public:

	/// Status values
	//@{
	/// Status ok.
	static const int OK/* = 1*/;

	/// Status failed.
	static const int FAILED/* = 0*/;

	//@}

	/// Return values from Read/Write methods.
	//@{
	/// Socket is not open.
	static const int SOCKET_NOT_OPEN/* = -1*/;

	/// The read/write timed out.
	static const int TIMED_OUT/* = -4*/;

	//@}

	QUANTAnet_udp_c();
        virtual ~QUANTAnet_udp_c() {}

	/** Set timeout period when performing read() or write() operations.
	    @param t Time out period in seconds. Set to	QUANTAnet_udp_c::NO_TIME_OUT to set no time out period.
	*/
	void setTimeOut(int t) {timeOutPeriod = t;}

	/// Get timeout period
	int getTimeOut() {return timeOutPeriod;}

	/** Open a socket.
	  * @param port Specifies port number.
	  * @return FAILED if failed. OK if succeeded.
	  * With the port parameter omitted a dynamic
	 * port will be established. Dynamic ports are typically
	 * used if you intend to be a client program sending packets to
	 * a server. If you are a server then you need to specify
	 * some specific port number. Port numbers must  be > 6000.
	 */
	int init(unsigned short port = 0);

	/** Set address to send to.
	 * After a socket is open, you can create a connection to
	 * some other listener if you know their address and port
	 @return 0 if failed, 1 if success.
	 */
	int setSendAddress(const char* host, unsigned short port);

	/** Send data.
      * @return The number of characters sent.
      * If a -1 is found then an error has occured.
      * Check the global variable errno. If you are using NON blocking 
      * and errno is EWOULDBLCOK then that's ok, it simply means that you
      * attempted a write but no the socket was not ready. If it is not 
      * EWOULDBLOCK then a real error has occured. If you are using blocking
      * with a timeout period (set by using the setTimeOut()),you will get an 
      * error if the timeout occurs.

      * @param length
      * If length is set to smaller than the actual outgoing packet size,
      * then the remaining bytes are not sent.
      * If the length is set to larger than actual packet size, the whole packet      * gets sent.

      * @param blockingType
      * Default value of this argument is QUANTAnet_udp_c::NULL_VALUE.
      * It can be set to QUANTAnet_udp_c::BLOCKING or 
      * QUANTAnet_udp_c::NON_BLOCKING.
      * If a value is not passed the blocking type is changed to the value
      * stored by the private memeber 'typeOfBlocking'.
      */
	int send(char* message, int length, int blockingType = QUANTAnet_udp_c::NULL_VALUE);

	/** Receive data.
	  * @return The number of bytes found in the packet.
	  * If a -1 is found then there might be a read error.
	  * Check the global variable errno. If you are using NON blocking
	  * and errno is EWOULDBLOCK then that's ok, it simply means you
	  * attempted a read but no data was available. If its not EWOULDBLOCK
	  * then you have a real error. To find out what the error means call:
	  * perror() - this is a UNIX call.If you are using blocking
      * with a timeout period (set by using the setTimeOut()),you will get an 
      * error if the timeout occurs.

	  * @param length
	  * If length is set to smaller than actual incoming packet size,
	  * then the remaining bytes of packet is discarded.
	  * If length is set to larger than actual packet size,
	  * then only the number of bytes available in the packet is retrieved.

      * @param blockingType
      * Default value of this argument is QUANTAnet_udp_c::NULL_VALUE.
      * It can be set to QUANTAnet_udp_c::BLOCKING or 
      * QUANTAnet_udp_c::NON_BLOCKING.
      * If a value is not passed the blocking type is changed to the value
      * stored by the private memeber 'typeOfBlocking'.
	  */
	int receive(char *message, int length,  int blockingType = QUANTAnet_udp_c::NULL_VALUE);


	/** Peek at incoming packet. - without extracting it from the queue.
	    @return number of bytes available to be extracted.
	*/
	int peek(char *message, int length);

	/// Make a connection non blocking (This method is being phased out of the distribution - you can set the blocking type in send() or receive() directly)
	void makeNonBlocking();

	/// Accessor Functions
	//@{
	/// Get an incoming packet's originating address.
	unsigned long getReceiveIP();
	/// Get an incoming packet's originating address.
	void getReceiveIP(char *ip);
	/// Get an incoming packet's originating port.
	unsigned short getReceivePort();

        /// Get outgoing packet's destination address.
	unsigned long getSendIP();
        /// Get outgoing packet's destination address.
	void getSendIP(char *ip);
        /// Get outgoing packet's destination port.
	unsigned short getSendPort();

	/// Get your own port.
	unsigned short  getSelfPort();

	/// Get the socket id
	int getSocketId();

        //@}


	/// Print information on the connection (IP address, port number etc).
	void printInfo();

	/** Close the udp handle.
	 * Must not do this as a destructor or you will disconnect a connection that
	 * might be still held by another client object. Only do this explicitly when you
	 * want to close a connection.
	 */
	int close();



    /**
	@param q_iovec QUANTAnet_iovec_c
	@param blockingType QUANTAnet_udp_c::BLOCKING or NON_BLOCKING
	@return 1 if succeeds or errno if fails
	comment:readv gives bad address error when iovec pointer is not initialized.
    */
    int readv(QUANTAnet_iovec_c* q_iovec);


    /**
        @param q_iovec QUANTAnet_iovec_c
        @param blockingType QUANTAnet_udp_c::BLOCKING or NON_BLOCKING
        @return 1 if succeeds or errno if fails
        comment:writev gives bad address error when iovec pointer is not initialized.
    */
    int writev(QUANTAnet_iovec_c* q_iovec);


};


#endif
