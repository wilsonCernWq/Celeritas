#include "SON_celeritasClient.h"
#include <climits>

namespace SON_PROTOCOL
{

SON_celeritasClient :: SON_celeritasClient () : m_tcpClient(0),
												m_tcpSockFd (-1),
												m_udpSockFd (-1),
												m_localUDPPort (-1),
												m_remoteUDPPort(-1),
												m_msgID(0)
{
}

SON_celeritasClient :: ~SON_celeritasClient ()
{
	if (m_tcpClient)
	{
		delete m_tcpClient;
		m_tcpClient = 0;
	}
}

int SON_celeritasClient :: setPayloadSize( int psize)
{
	m_payloadSize = psize;

	m_payloadDataSize = m_payloadSize -
						SON_PROTOCOL::CELERITAS_HDR_ELEM * sizeof(unsigned int);

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: setRate ( int rate_in_mbps)
{

	if (m_payloadSize == -1)
	{
		m_sleepIntervalUsec =  1000;
		return SON_PROTOCOL::SUCCESS;
	}
	double pps = (double) (rate_in_mbps ) * (1000) *  (1000) / ( 8 * m_payloadDataSize);
	double sleeptime_usec = (1 / pps) * (1000) * 1000 ;

	// m_sleepIntervalUsec =   (int) ceil(sleeptime_usec);
	m_sleepIntervalUsec =   (int) ceil(sleeptime_usec);

	cout << " Sleep TIme is in Microsec is " << sleeptime_usec <<  endl;


	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient::connectToServer ( const char* ip, int port)
{
	char destnport[1024];
	memset (destnport, '\0', 1024);
	sprintf(destnport, "%d", port);

	return this->connectToServer(ip, destnport);
}

int SON_celeritasClient::connectToServer ( const char* ip, char* port)
{
	m_tcpClient =  new SON_tcpClient();

	if (SON_PROTOCOL::FAILURE == m_tcpClient->connectToServer (ip, port))
	{
		delete m_tcpClient;
		m_tcpClient = 0;

		return SON_PROTOCOL::FAILURE;
	}

	// Set the TCP Socket FD
	m_tcpSockFd = m_tcpClient->getSockFd();

	// Create the UDP Socket
	if (SON_PROTOCOL::SUCCESS != this->__createUDPSocket())
		return SON_PROTOCOL::FAILURE;

	// Init Local UDP Port
	if (SON_PROTOCOL::SUCCESS != this->__initLocalUDPPort())
		return SON_PROTOCOL::FAILURE;

	// Send the Local UDP Port
	if (SON_PROTOCOL::SUCCESS != this->__sendLocalUDPPort())
		return SON_PROTOCOL::FAILURE;

	// Read the Remote UDP Port
	if (SON_PROTOCOL::SUCCESS != this->__readRemoteUDPPort())
		return SON_PROTOCOL::FAILURE;

	// Connect the UDP Socket
	if (SON_PROTOCOL::SUCCESS != this->__connectUDPSocket())
		return SON_PROTOCOL::FAILURE;

	// Set UDP socket NON BLOCKING
	if (SON_PROTOCOL::SUCCESS != this->__setUDPNonBlocking())
			return SON_PROTOCOL::FAILURE;


	// \NOTE \TODO \FIXME Hardcoded to 16M ... Remove later

	int bufsize =  4 * 1024 * 1024;
	this->setSendBufSize(bufsize);
	this->setRecvBufSize(bufsize);


	this->initPerf();

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: close ()
{
	m_tcpClient->close();

	::close ( m_udpSockFd);

	return SON_PROTOCOL::SUCCESS;
}


/*
* 1.	Compute the number of packet to be received
* 2.	Create the Bitset list of of packets
* 3.	Read the main list of data packets
* 4.	Read the main done TCP packet
* 5.	Send the error list or done list to sender
* 6. 	If there are packet errors then receive the remaining packets
*/


int  SON_celeritasClient :: read (char *ptr, long long& nbytes)
{
	// ------------------------------------------------------------------------
	// Compute the number of packets to be received
	// Initialize the Error List Bitset
	// ------------------------------------------------------------------------
	unsigned int numpackets = (int) ceil ((double) nbytes /
									(double) m_payloadDataSize);

	long long bytes_read = 0; // The total bytes read
	unsigned int lost_packets = 0;

	// ------------------------------------------------------------------------
	// Increment the Message ID
	// ------------------------------------------------------------------------
	unsigned int msgid = m_msgID;
	this->__incMsgID();

	// ------------------------------------------------------------------------
	// 	The error list
	//	We set the error list as zero. On a packet loss we set the bit as 1
	// ------------------------------------------------------------------------
	vector < bitset <32> > error_list;
	unsigned int error_list_size = numpackets / 32 ;
	if (numpackets % 32)
		error_list_size++;
	error_list.reserve(error_list_size);
	for (unsigned int i = 0; i <  error_list_size; i++)
	{
		error_list.push_back(0);
	}

	// The Error Array List
	unsigned int loss_array_size = error_list_size +
											SON_PROTOCOL::CELERITAS_HDR_ELEM;
	unsigned int loss_array [ loss_array_size];

	bool successful_reception = false;

	unsigned int first_packet  = 0;

	// ------------------------------------------------------------------------
	// 	Read the main sequence of packets.. This could be zero copy or use a
	//	temporary buffer
	// ------------------------------------------------------------------------
	if (SON_PROTOCOL::SUCCESS != this->__bufferedRead (ptr, bytes_read, msgid,
											first_packet, numpackets, error_list,
											error_list_size, lost_packets))
	{
		nbytes = bytes_read;
		return SON_PROTOCOL::FAILURE;
	}

	// ------------------------------------------------------------------------
	//	1. If we have lost packets, Send the error list to the remote server
	//	2. Read the Lost Packets from the remote sender
	//	3. If we have received all packets then exit else GOTO 1
	// ------------------------------------------------------------------------

	while (!successful_reception)
	{
		if (SON_PROTOCOL::SUCCESS != this->__sendErrorList ( msgid, error_list,
												error_list_size, loss_array,
												loss_array_size, lost_packets))
		{
			nbytes = bytes_read;
			return SON_PROTOCOL::FAILURE;
		}

		if (0 == lost_packets)
		{
			successful_reception =  true;
			break;
		}
		else
		{
			// Read the Lost packets
			if (SON_PROTOCOL::SUCCESS != this->__recvLostData (ptr, bytes_read,
											msgid, error_list, error_list_size,
											loss_array, loss_array_size,
											lost_packets))
			{
				nbytes = bytes_read;
				return SON_PROTOCOL::FAILURE;
			}
		}
	}

	// Update Perf Info
	double val = (double) bytes_read;
	m_perfDB.updateDataRead(val);

	nbytes = bytes_read;
	return SON_PROTOCOL::SUCCESS;

}

int  SON_celeritasClient :: write ( const char *ptr, long long& nbytes)
{
	// ------------------------------------------------------------------------
	// Compute the number of packet to be received and initialize
	// the Error List Vector
	// ------------------------------------------------------------------------
	unsigned int numpackets = (unsigned int) ceil ((double) nbytes /
										(double) m_payloadDataSize);

	const long long buf_size = nbytes;

	long long totwrite = 0, nwrite, nread;

	// ------------------------------------------------------------------------
	// Increment the Message ID
	// ------------------------------------------------------------------------
	unsigned int msgid = m_msgID;
	this->__incMsgID();

	// ------------------------------------------------------------------------
	// The error list
	// ------------------------------------------------------------------------
	vector < bitset <32> > error_list;
	unsigned int error_list_size = numpackets / 32 ;
	if (numpackets % 32)
		error_list_size++;
	error_list.reserve(error_list_size);

	// The 1 is for the # of errors
	unsigned int loss_array_size = error_list_size +
											SON_PROTOCOL::CELERITAS_HDR_ELEM;
	unsigned int loss_array [loss_array_size];

	unsigned int tot_packets = 0, first_packet, payloadsize;

	// Send the main set of packets .... we send the last packet separately
	first_packet = 0;
	tot_packets = numpackets - 1;
	payloadsize = m_payloadDataSize;

	if (SON_PROTOCOL::SUCCESS !=
			this->__writeData(ptr, buf_size, nwrite, msgid, payloadsize,
								first_packet, tot_packets ))
		return SON_PROTOCOL::FAILURE;

	totwrite += nwrite;

	// Send the last packet
	first_packet = numpackets - 1;
	tot_packets = 1;
	payloadsize = nbytes - ((numpackets - 1) * m_payloadDataSize);

	if ( SON_PROTOCOL::SUCCESS !=
			this->__writeData(ptr, buf_size, nwrite,msgid, payloadsize,
								first_packet,tot_packets))
		return SON_PROTOCOL::FAILURE;

	totwrite += nwrite;

	// ------------------------------------------------------------------------
	// A forced scheduler yield to let all UDP packets be flushed out
	// The message sequencing scheme recovers from all situations and this is
	// just a preventive step i.e. an optimization
	// ------------------------------------------------------------------------
	this->__usecDelay(m_sleepIntervalUsec);

	// Send TCP done
	long long endmsg_size =  sizeof(int);
	int endmsg = 0;

	if (SON_PROTOCOL::SUCCESS !=  m_tcpClient->write ((char*) &endmsg, endmsg_size))
		return SON_PROTOCOL::FAILURE;

	// ------------------------------------------------------------------------
	//	Error Correction
	//	1. Read the error list from the remote client
	//	2. If the num of errors are zero, GOTO 4
	//	3.	We have errors and need to resend lost packets
	//	3a.	From the error list received, create the error bitset
	//	3b.	For each "set" element in the error bitset, send the packet
	//	3c.	Goto 1
	//	4. Return (We are done sending successfully)
	// ------------------------------------------------------------------------

	bool send_successful = false;
	unsigned int lost_packets;

	while (!send_successful)
	{

		// Receive feedback from receiver if there are any errors....
		nread = loss_array_size * sizeof(unsigned int);

		if (SON_PROTOCOL::SUCCESS !=  m_tcpClient->read ((char*) loss_array, nread))
			return SON_PROTOCOL::FAILURE;

		lost_packets = loss_array[1];
		cout << "  The Total Lost packets are " << lost_packets<<  endl;

		// No Errors...
		if (0 == lost_packets)
		{
			send_successful = true;
			break;
		}

		// --------------------------------------------------------------------
		// 	We have errors and need to perform error detection and correction
		// 	1. Prepare the  Error List to send to the sender
		//	2. Send the list to the Sender
		//	3. Receive the Lost packets
		//	4. Check if there are still any losses. If yes GOTO 1, else 5
		//	5. Exit.
		// --------------------------------------------------------------------

		// Retransmit Lost packets
		 if (SON_PROTOCOL::SUCCESS != this->__retransmitErrorBitList (ptr,
														buf_size,
														nwrite,
														msgid,
														error_list,
														error_list_size,
														loss_array,
														loss_array_size,
														lost_packets))
		 {

			return SON_PROTOCOL::FAILURE;
		 }
	}

	// Update Performance Data
	double val = (double) totwrite;
	m_perfDB.updateDataWritten(val);

	nbytes =  totwrite;

	return SON_PROTOCOL::SUCCESS;
}



int SON_celeritasClient :: __retransmitErrorBitList (const char* ptr,
											const long long& msg_size,
											long long& totwrite,
											unsigned int& msgid,
											vector < bitset <32> >& error_list,
											unsigned int& error_list_size,
											unsigned int* loss_array,
											unsigned int& loss_array_size,
											unsigned int& lost_packets)
{

	unsigned int numpackets = (unsigned int) ceil ((double) msg_size /
											(double) m_payloadDataSize);

	// We have errors to process
	// Prepare the  Error List
	error_list.clear();
	for (unsigned int i = 0; i < error_list_size; i++)
	{
		error_list.push_back(loss_array[i+SON_PROTOCOL::CELERITAS_HDR_ELEM]);
	}

	unsigned int first_packet;
	unsigned int tot_packets = 1;
	unsigned int payloadsize = m_payloadDataSize;


	// Send all the Lost packets
	for (unsigned int i = 0; i < error_list_size; i++)
	{
		if (!(error_list[i].any()))
			continue;

		for (unsigned int j = 0; j < 32 ; j++)
		{
			if (error_list[i][j])
			{
				// Lost Packet ... Need to resend this packet

				first_packet = (i * 32) +  j;

				if (first_packet == (numpackets -1))
				{
					payloadsize = msg_size -
								((numpackets - 1) * m_payloadDataSize);
				}

				if ( SON_PROTOCOL::SUCCESS !=
					this->__writeData(ptr, msg_size, totwrite,msgid, payloadsize,
									first_packet,tot_packets))

					return SON_PROTOCOL::FAILURE;
			}
		}
	}

	// Send TCP End of Transmission
	long long endmsg_size =  sizeof(int);
	int endmsg = 0;

	if (SON_PROTOCOL::SUCCESS !=
					m_tcpClient->write ((char*) &endmsg, endmsg_size))
		return SON_PROTOCOL::FAILURE;

	return SON_PROTOCOL::SUCCESS;
}




void SON_celeritasClient :: __incMsgID (void)
{
	// Need to take care of overflows....

    if (m_msgID == (UINT_MAX - 1))
    {
        m_msgID = 0;
        return;
    }

    m_msgID = (m_msgID + 1) % (UINT_MAX);
}

void SON_celeritasClient :: printConnectionInfo (void)
{
	cout << " ---------- Connection Information ---------- " << endl;

    // Retrive the IP using the TCP socket
    char localip [1024];
	memset (localip, '\0', 1024);
	if (SON_PROTOCOL::FAILURE ==  m_tcpClient->getSelfIP(localip))
		cout << " ------ ERROR WITH CONNECTION -------- " << endl;
	cout << " LOCAL IP IS : " << localip << endl;

	// Retrive the Remote IP using the TCP socket
	char remoteip [1024];
	memset (remoteip, '\0', 1024);
	if (SON_PROTOCOL::FAILURE ==  m_tcpClient->getRemoteIP(remoteip))
	{
		cout << " ------ ERROR WITH CONNECTION -------- " << endl;
	}
	cout << " Remote IP IS : " << remoteip << endl;

	cout << " Local UDP Port : " << m_localUDPPort << endl;
	cout << " Remote UDP Port : " << m_remoteUDPPort << endl;

	cout << " RECV Buf Size is " << this->getRecvBufSize() <<
			" and Send BUf is " << this->getSendBufSize() << endl;

	cout << " -------------------------------------------- " << endl;
}

void  SON_celeritasClient :: initPerf (void)
{
	m_perfDB.initConnStartTime();
}

int SON_celeritasClient :: setRecvBufSize (int size)
{
	if (::setsockopt(this->m_udpSockFd,SOL_SOCKET,SO_RCVBUF, \
					(char *) &size, sizeof(int)) < 0)
		return SON_PROTOCOL::FAILURE;

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: getRecvBufSize (void)
{
	int retsize = -1, retsizelen = sizeof(int);
	if (::getsockopt(this->m_udpSockFd,SOL_SOCKET,SO_RCVBUF, \
							(char *) &retsize,(socklen_t*) &retsizelen) < 0)
	{
		SON_PRINT_ERROR(" GetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}

	return retsize;
}

int SON_celeritasClient :: setSendBufSize (int size)
{
	if (::setsockopt(this->m_udpSockFd,SOL_SOCKET,SO_SNDBUF, \
					(char *) &size, sizeof(int)) < 0)
		return SON_PROTOCOL::FAILURE;

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: getSendBufSize (void)
{
	int retsize = -1, retsizelen = sizeof(int);
	if (::getsockopt(this->m_udpSockFd,SOL_SOCKET,SO_SNDBUF, \
							(char *) &retsize,(socklen_t*) &retsizelen) < 0)
	{
		SON_PRINT_ERROR(" GetsockOpt :: %d : %s \n ", \
				__LINE__, __FILE__);
		retsize = -1;
	}

	return retsize;
}

double SON_celeritasClient :: getReadBW(void)
{
	return m_perfDB.getReadBW();
}

double SON_celeritasClient :: getWriteBW(void)
{
	return m_perfDB.getWriteBW();
}

double SON_celeritasClient :: getTotalBW(void)
{
	return m_perfDB.getTotalBW();
}

double SON_celeritasClient :: getDataRead(void)
{
	return m_perfDB.getDataRead();
}

double SON_celeritasClient :: getDataWritten(void)
{
	return m_perfDB.getDataWritten();
}

struct timeval SON_celeritasClient :: getConnStartTime(void)
{
	return m_perfDB.getConnStartTime();
}

int SON_celeritasClient :: __setUDPNonBlocking (void)
{
	int val =  fcntl(m_udpSockFd, F_GETFL, 0);
	if (val < 0)
	{
		perror(" fcntl : " );
		return SON_PROTOCOL::FAILURE;
	}

	if (fcntl(m_udpSockFd, F_SETFL, val | O_NONBLOCK) == -1)
	{
		perror(" FCNTL :  ");
		return SON_PROTOCOL::FAILURE;
	}

	return SON_PROTOCOL::SUCCESS;
}

void SON_celeritasClient :: __setTCPClient (SON_tcpClient* client)
{
	m_tcpClient = client;

	// Set the TCP Socket FD
	m_tcpSockFd = m_tcpClient->getSockFd();
}

int SON_celeritasClient :: __initLocalUDPPort (void)
{
	int port = -1;

	port = this->__getLocalUDPPort();

	if (SON_PROTOCOL::FAILURE == port)
		return SON_PROTOCOL::FAILURE;

	m_localUDPPort = port;

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: __sendLocalUDPPort(void)
{
	long long nwrite = sizeof(int);
	return m_tcpClient->write((const char*) &m_localUDPPort, nwrite);
}

int SON_celeritasClient :: __readRemoteUDPPort(void)
{
	long long nread = sizeof(int);
	int rport;

	if (SON_PROTOCOL::SUCCESS == m_tcpClient->read(( char*) &rport, nread) )
	{
		m_remoteUDPPort = rport;
		return SON_PROTOCOL::SUCCESS;
	}
	else
		return SON_PROTOCOL::FAILURE;
}

int SON_celeritasClient :: __createUDPSocket (void)
{
    struct addrinfo hints, *res, *ressave;
    int error, sockfd;

    memset(&hints, 0, sizeof(struct addrinfo));

    // Retrive the IP using the TCP socket
    char localip [1024];
	memset (localip, '\0', 1024);
	if (SON_PROTOCOL::FAILURE ==  m_tcpClient->getSelfIP(localip))
		return SON_PROTOCOL::FAILURE;

	// ---------------------------------------------------------
    //   AI_PASSIVE flag: the resulting address is used to bind
    //   to a socket for accepting incoming connections.
    //   So, when the hostname==NULL, getaddrinfo function will
    //   return one entry per allowed protocol family containing
	//   the unspecified address for that family.
	// ---------------------------------------------------------

    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    error = getaddrinfo(localip, 0, &hints, &res);
    if (error != 0) {
        fprintf(stderr,
                "getaddrinfo error:: [%s]\n",
                gai_strerror(error));
        return SON_PROTOCOL::FAILURE;
    }

    ressave = res;

    // ---------------------------------------------------------
    // Try open socket with each address getaddrinfo returned,
    // until getting a valid listening socket.
    // ---------------------------------------------------------

    sockfd=-1;
    do {
        sockfd = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol);

        if (sockfd >= 0)
        {
            if (::bind (sockfd, res->ai_addr, res->ai_addrlen) == 0)
              break;

            ::close(sockfd);
            sockfd = -1;
        }

    } while ((res = res->ai_next) != NULL);

	::freeaddrinfo(ressave);

    if ((sockfd < 0)|| (res == NULL))
    {
        fprintf(stderr,
                "socket error:: could not open socket\n");
        return SON_PROTOCOL::FAILURE;
    }

	// Creation of socket was a success.. Assign the UDP socket FD
	m_udpSockFd =  sockfd;

	cout << " LOCAL UDP PORT IS : " << this->__getLocalUDPPort();

    return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: __connectUDPSocket (void)
{
    struct addrinfo hints, *res, *ressave;
    int error;

    memset(&hints, '\0', sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

	char remoteip [1024];
	memset (remoteip, '\0', 1024);
	if (SON_PROTOCOL::FAILURE ==  m_tcpClient->getRemoteIP(remoteip))
		return SON_PROTOCOL::FAILURE;

	char remoteport[1024];
	memset (remoteport, '\0', 1024);
	sprintf( remoteport,"%d", m_remoteUDPPort);

	cout << " Remote UDP Port is  " << m_remoteUDPPort <<  endl;


	error = getaddrinfo(remoteip,remoteport, &hints, &res);
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
    	if (::connect(m_udpSockFd, res->ai_addr, res->ai_addrlen) == 0)
		{
        	 break;
		}
		res = res->ai_next;
    }

    int retval;
    if (0 == res)
        retval = SON_PROTOCOL::FAILURE;
    else
		retval = SON_PROTOCOL::SUCCESS;

	::freeaddrinfo(ressave);

    return retval;
}

int SON_celeritasClient :: __getLocalUDPPort ()
{
    int retport;
    int retval;
    struct sockaddr_storage addr;
    socklen_t addr_len =  sizeof(struct sockaddr_storage) ;
    char myport[1024];

    retval = ::getsockname(m_udpSockFd,(struct sockaddr *) &addr, &addr_len);
    if (retval < 0)
    {
        perror( " getRemotePort :: getsockname : " );
        return SON_PROTOCOL::FAILURE;
    }

    retval =  ::getnameinfo( (const struct sockaddr *) &addr, addr_len,
								0, 0, myport, 1024, NI_NUMERICSERV | NI_DGRAM );
    if (retval < 0)
    {
        perror( "  getRemotePort ::getnameinfo : " );
        return SON_PROTOCOL::FAILURE;
    }

    retport = atoi(myport);
    return retport;
}

void SON_celeritasClient :: __usecDelay ( int& usec)
{
	struct timeval ts;

	ts.tv_sec = 0;
	ts.tv_usec = usec;

	::select(1, 0, 0, 0, &ts);
}

void SON_celeritasClient :: __msecDelay ( int& msec)
{
	struct timeval ts;

	ts.tv_sec = 0;
	ts.tv_usec = msec * 1000;

	::select(1, 0, 0, 0, &ts);
}

int  SON_celeritasClient :: __writeData ( const char *ptr,
										const long long& buf_size,
										long long& bytes_sent,
										unsigned int& msgid,
										unsigned int& payloadsize,
										unsigned int& firstpacketid,
										unsigned int& numpackets)
{
	struct iovec send_list[2];
	struct msghdr datahdr;
	fd_set fdset;

	int select_fd, myerr;

	unsigned int headersize = SON_PROTOCOL::CELERITAS_HDR_ELEM
											* sizeof(unsigned int);
	unsigned int cur_packet = firstpacketid;
	long long totwrite = 0;
	long long numsent = 0;

	unsigned int last_packet = firstpacketid + numpackets -1;

	unsigned int hdr[2];
	hdr[0] = msgid;

	datahdr.msg_name = 0;
	datahdr.msg_namelen = 0;
	datahdr.msg_iov = send_list;
	datahdr.msg_iovlen = 2;
	datahdr.msg_control = 0;
	datahdr.msg_controllen = 0;
	send_list[0].iov_base = hdr;

	FD_ZERO( &fdset);

	while ( cur_packet <= last_packet )
	{
		hdr[1] = cur_packet;
		send_list[0].iov_len = headersize;

		send_list[1].iov_base = (void *) &ptr[cur_packet * m_payloadDataSize];
		send_list[1].iov_len = payloadsize;

		FD_SET(m_udpSockFd, &fdset);

		select_fd = select(m_udpSockFd + 1, 0, &fdset,0,0);
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
	        		bytes_sent =  totwrite;
	        		return SON_PROTOCOL::FAILURE;
	        		break;
	        }
		} // end of select

		if (FD_ISSET(m_udpSockFd, &fdset))
		{
			numsent = ::sendmsg (m_udpSockFd, &datahdr, 0);
			if (numsent > 0)
			{
				// Increment sent packets
				++cur_packet;
				totwrite += numsent - headersize;

				// Sleep for the time interval (Rate Control)
				if (cur_packet % 1024)
					__usecDelay_RDTSC (m_sleepIntervalUsec);
				else
					__usecDelay (m_sleepIntervalUsec);
			}

			else if (-1  == numsent)
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
						bytes_sent =  totwrite;
						return SON_PROTOCOL::FAILURE;
						break;
	            }
			}
		} // if FD_ISSET

	} // end of while


	bytes_sent = totwrite;

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: __sendErrorList (unsigned int& msgid,
											vector < bitset <32> >& error_list,
											unsigned int& error_list_size,
											unsigned int* loss_array,
											unsigned int& loss_array_size,
											unsigned int& lost_packets)
{
	// ------------------------------------------------------------------------
	//	The integer array to send the error_list and the control data
	//	The header includes MSGID and # of error packets
	// ------------------------------------------------------------------------
	long long nwrite =  loss_array_size * sizeof(unsigned int);

	memset(loss_array, '\0', nwrite);

	loss_array[0] = msgid;
	loss_array[1] = lost_packets;

	if (lost_packets)
	{
		// Convert the bitset vector into an array
		for (unsigned int i = 0;  i < error_list.size(); i++)
		{
			loss_array[i+2] = error_list[i].to_ulong();
		}
	}

	if (SON_PROTOCOL::FAILURE == m_tcpClient->write((char*)loss_array, nwrite))
		return SON_PROTOCOL::FAILURE;

	SON_PRINT_DEBUG1( "Sent Loss List with %u packets \n", lost_packets);

	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: __recvLostData (	char* ptr,
											long long& bytes_read,
											unsigned int& msgid,
											vector < bitset <32> >& error_list,
											unsigned int& error_list_size,
											unsigned int* loss_array,
											unsigned int& loss_array_size,
											unsigned int& lost_packets)
{
	// ---------------------------------------------------------
	// 1. Prepare the DS for receiving the data
	// 2. Receive the Data
	// 3. DATA could be a UDP data packet or a TCP control packet
	// 3a. If its a TCP control packet then reply back with error list
	// 3b. if its a UDP control packet then update the error list
	// ---------------------------------------------------------

	fd_set fdset;

	int max_fd = m_udpSockFd;
	if (m_udpSockFd < m_tcpSockFd)
		max_fd = m_tcpSockFd;

	unsigned int cur_packet;
	unsigned int payloadsize = m_payloadDataSize;
	unsigned int headersize = SON_PROTOCOL::CELERITAS_HDR_ELEM
												* sizeof(unsigned int);

	unsigned int hdr[2];

	int select_fd, myerr;

	// The amount of total data read and read in the current recvmsg
	long long totread = 0, nread;

	// The last buffer packet can be less than the payloadsize
	unsigned char temp_buf [m_payloadDataSize];

	//  TCP end of Message
	unsigned int endmsg = 0;
	bool end_of_transmission = false;

	struct msghdr data_hdr;
	struct iovec recv_list[2];

	data_hdr.msg_name = 0;
	data_hdr.msg_namelen = 0;
	data_hdr.msg_iov = recv_list;
	data_hdr.msg_iovlen = 2;
	data_hdr.msg_control = 0;
	data_hdr.msg_controllen = 0;

	recv_list[0].iov_base = hdr;
	recv_list[0].iov_len = headersize;

	recv_list[1].iov_base =  temp_buf;
	recv_list[1].iov_len = payloadsize;

	FD_ZERO( &fdset);

	while ( !end_of_transmission)
	{
		FD_SET(m_udpSockFd, &fdset);
		FD_SET(m_tcpSockFd, &fdset);

		// Wait for a socket FD to become active
		select_fd = ::select(max_fd + 1, &fdset,0, 0, 0);
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

					bytes_read +=  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}


		// UDP Data Packet
		if (FD_ISSET(m_udpSockFd, &fdset))
		{
			nread = ::recvmsg (m_udpSockFd, &data_hdr, 0);

			if (nread > 0)
			{
				// Check if its remnants of a previous message
				if (hdr[0] != msgid)
					continue;

				cur_packet = hdr[1];

				memcpy (&ptr[cur_packet * payloadsize], temp_buf, nread - headersize);

				// If packet is in the lost list, remove it
				if (error_list [ cur_packet/32][cur_packet%32])
				{
					error_list [ cur_packet/32][cur_packet%32] = 0;
					lost_packets--;

					totread +=  nread - headersize ;
				}

			} // end of if nread > 0

			else if (0 == nread)
				continue;
			else
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

						bytes_read +=  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}

		}

		// TCP Control Packet
		if (FD_ISSET(m_tcpSockFd, &fdset))
		{
			// prepare the header for the TCP message
			recv_list[0].iov_base = &endmsg;
			recv_list[0].iov_len = sizeof(unsigned int);
			data_hdr.msg_name = 0;
			data_hdr.msg_namelen = 0;
			data_hdr.msg_iov = recv_list;
			data_hdr.msg_iovlen = 1;
			data_hdr.msg_control = 0;
			data_hdr.msg_controllen = 0;

			nread = ::recvmsg(m_tcpSockFd, &data_hdr, 0);

			if (nread < 0)
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
						bytes_read  +=  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}

			if (0 == endmsg)
			{
				end_of_transmission =  true;
				cout << " Received TCP ENd of Transmission " <<  endl;
			}
		}
	}

	SON_PRINT_DEBUG1(" Packets Remaining are %u \n", lost_packets);

	bytes_read += totread;
	return SON_PROTOCOL::SUCCESS;
}

int SON_celeritasClient :: __bufferedRead (	char* ptr,
											long long& nbytes,
											unsigned int& msgid,
											unsigned int& first_packet,
											unsigned int& totpackets,
											vector < bitset <32> >& error_list,
											unsigned int& error_list_size,
											unsigned int& lost_packets)
{
	// ---------------------------------------------------------
	// 1. Prepare the DS for receiving the data
	// 2. Receive the Data
	// 3. DATA could be a UDP data packet or a TCP control packet
	// 3a. If its a TCP control packet then reply back with error list
	// 3b. if its a UDP control packet then update the error list
	// ---------------------------------------------------------

	struct iovec recv_list[2];
	struct msghdr data_hdr;
	fd_set fdset;

	int max_fd = m_udpSockFd;
	if (m_udpSockFd < m_tcpSockFd)
		max_fd = m_tcpSockFd;

	// Current Received packet and expected packet
	unsigned int cur_packet;
	unsigned int expected_packet = first_packet;
	unsigned int max_recvd_packet = 0;
	unsigned int last_packet = (totpackets  - 1);

	unsigned int payloadsize = m_payloadDataSize;
	unsigned int headersize = SON_PROTOCOL::CELERITAS_HDR_ELEM
									* sizeof(unsigned int);

	int select_fd, myerr;

	bool end_of_transmission =  false;
	unsigned int endmsg;

	// The amount of total data read and read in the current recvmsg
	long long totread = 0, nread;

	// The last buffer packet can be less than the payloadsize
	unsigned char temp_buf [m_payloadDataSize];

	data_hdr.msg_name = 0;
	data_hdr.msg_namelen = 0;
	data_hdr.msg_iov = recv_list;
	data_hdr.msg_iovlen = 2;
	data_hdr.msg_control = 0;
	data_hdr.msg_controllen = 0;

	unsigned int hdr[2];

	recv_list[0].iov_base = hdr;
	recv_list[0].iov_len = headersize;

	recv_list[1].iov_base =  temp_buf;
	recv_list[1].iov_len = payloadsize;

	FD_ZERO( &fdset);

	while ( !end_of_transmission)
	{
		FD_SET(m_udpSockFd, &fdset);
		FD_SET(m_tcpSockFd, &fdset);

		select_fd = ::select(max_fd + 1, &fdset,0, 0, 0);
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

					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}


		// UDP Data Packet
		if (FD_ISSET(m_udpSockFd, &fdset))
		{
			nread = ::recvmsg (m_udpSockFd, &data_hdr, 0);

			if (nread > 0)
			{
				// Check if its remnants of a previous message
				if (hdr[0] != msgid)
					continue;

				cur_packet = hdr[1];

				memcpy (&ptr[cur_packet * payloadsize], temp_buf, nread - headersize);

				// Successfully received data.. Now check if there are any losses
				if  ( cur_packet > expected_packet)
				{
					cout << " Packet Loss --- Expected " << expected_packet <<  " Received " << cur_packet <<  endl;

					// Increment the total number of lost packets
					lost_packets += cur_packet - expected_packet ;

					// Update the loss list from expected_packet to cur_packet - 1
					for (unsigned int i = expected_packet; i < cur_packet; i++)
					{
						error_list[i/32].set(i%32, 1);
					}
				}

				if (cur_packet < expected_packet)
					cout << " **** Need to take care of this CASE **** " <<  endl;

				if (max_recvd_packet < cur_packet)
					max_recvd_packet = cur_packet;
				++cur_packet;
				expected_packet = cur_packet;
				totread +=  nread - headersize ;

			} // end of if nread > 0

			else if (0 == nread)
				continue;
			else
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

						nbytes =  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}

		}

		// TCP Control Packet
		if (FD_ISSET(m_tcpSockFd, &fdset))
		{
			// prepare the header for the TCP message
			recv_list[0].iov_base = &endmsg;
			recv_list[0].iov_len = sizeof(unsigned int);
			data_hdr.msg_name = 0;
			data_hdr.msg_namelen = 0;
			data_hdr.msg_iov = recv_list;
			data_hdr.msg_iovlen = 1;
			data_hdr.msg_control = 0;
			data_hdr.msg_controllen = 0;

			nread = ::recvmsg(m_tcpSockFd, &data_hdr, 0);

			if (nread < 0)
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
						nbytes =  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}

			// ----------------------------------------------------------------
			//	We may have received End of Transmission, however, there may be
			//	packets lost that may not be accounted for...
			// 	This list is from the max_recvd_packet to last_packet
			//	We update the loss_list if needed
			// ----------------------------------------------------------------

			if (0 == endmsg)
			{
				// We may have lost packets and received an End message....
				if (max_recvd_packet != last_packet)
				{
					cout << " END Packet Loss " << max_recvd_packet + 1  <<
							" TO : " << last_packet <<  endl;

					// Increment the total number of lost packets
					lost_packets += last_packet -  max_recvd_packet  ;

					for (unsigned int i = (max_recvd_packet + 1) ; i <= last_packet; i++)
					{
						error_list[i/32].set( i % 32, 1);
					}
				}

				end_of_transmission =  true;
				cout << " Received TCP ENd of Transmission " <<  endl;
			}
		}

	}

	nbytes = totread;
	cout << "  Total Lost packets = " << lost_packets <<  endl;
	return SON_PROTOCOL::SUCCESS;
}



int  SON_celeritasClient :: __zeroCopyRead ( char* ptr,
											long long& nbytes,
											unsigned int& msgid,
											unsigned int& first_packet,
											unsigned int& totpackets,
											vector < bitset <32> >& error_list,
											unsigned int& error_list_size,
											unsigned int& lost_packets)
{

	struct iovec recv_list[2];
	struct msghdr data_hdr;
	fd_set fdset;

	int max_fd = m_udpSockFd;
	if (m_udpSockFd < m_tcpSockFd)
		max_fd = m_tcpSockFd;

	// Current Received packet and expected packet
	unsigned int cur_packet;
	unsigned int expected_packet = first_packet;
	unsigned int max_recvd_packet = 0;
	unsigned int last_packet = (totpackets  - 1);

	unsigned int payloadsize = m_payloadDataSize;
	unsigned int headersize = SON_PROTOCOL::CELERITAS_HDR_ELEM
									* sizeof(unsigned int);

	int select_fd, myerr;

	bool end_of_transmission =  false;
	unsigned int endmsg;

	// The amount of total data read and read in the current recvmsg
	long long totread = 0, nread;

	bool last_buf_packet = false;

	data_hdr.msg_name = 0;
	data_hdr.msg_namelen = 0;
	data_hdr.msg_iov = recv_list;
	data_hdr.msg_iovlen = 2;
	data_hdr.msg_control = 0;
	data_hdr.msg_controllen = 0;

	FD_ZERO( &fdset);

	unsigned int hdr[2];
	recv_list[0].iov_base = hdr;
	recv_list[0].iov_len = headersize;
	recv_list[1].iov_len = payloadsize;

	// The last buffer packet can be less than the payloadsize
	unsigned char temp_buf [m_payloadDataSize];

	while ( !end_of_transmission)
	{
		FD_SET(m_udpSockFd, &fdset);
		FD_SET(m_tcpSockFd, &fdset);

		last_buf_packet =  false;

		// ZERO COPY
		recv_list[1].iov_base = &ptr[expected_packet * payloadsize];

		if (expected_packet == last_packet)
		{
			last_buf_packet =  true;
			recv_list[1].iov_base = temp_buf;
		}

		select_fd = ::select(max_fd + 1, &fdset,0, 0, 0);
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

					nbytes =  totread;
					return SON_PROTOCOL::FAILURE;
					break;
			}
		}


		// UDP Data Packet
		if (FD_ISSET(m_udpSockFd, &fdset))
		{
			nread = ::recvmsg(m_udpSockFd, &data_hdr, 0);

			if (nread > 0)
			{
				// Check if its remnants of a previous message
				if (hdr[0] != msgid)
					continue;

				cur_packet = hdr[1];

				// Successfully received data.. Now check if there are any losses
				if  ( cur_packet == expected_packet)
				{

					// Copy the last packet to its location
					if (last_buf_packet ==  true)
					{
						memcpy (&ptr[cur_packet * m_payloadDataSize],
											temp_buf, nread - headersize);
						last_buf_packet =  false;
					}
				}
				else
				{
					// Packet loss... Move data to the appropriate location
					if (!last_buf_packet)
					{
						memcpy (&ptr[cur_packet * m_payloadDataSize],
								&ptr[expected_packet * m_payloadDataSize],
								nread - headersize);
					}
					else
					{
						memcpy (&ptr[cur_packet * m_payloadDataSize],
											temp_buf, nread - headersize);
						last_buf_packet =  false;
					}


					cout << " Packet Loss --- Expected " << expected_packet << " Received " << cur_packet <<  endl;

					// Increment the total number of lost packets
					lost_packets += cur_packet - expected_packet ;

					// Update the loss list from expected_packet to cur_packet - 1
					for (unsigned int i = expected_packet; i < cur_packet; i++)
					{
						error_list[i/32].set(i%32, 1);
					}
				}

				if (cur_packet < expected_packet)
					cout << " **** Need to take care of this CASE **** " <<  endl;

				if (max_recvd_packet < cur_packet)
					max_recvd_packet = cur_packet;
				++cur_packet;
				expected_packet = cur_packet;
				totread +=  nread - headersize ;

			} // end of if nread > 0

			else if (0 == nread)
				continue;
			else
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

						nbytes =  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}

		}

		// TCP Control Packet
		if (FD_ISSET(m_tcpSockFd, &fdset))
		{
			// prepare the header for the TCP message
			recv_list[0].iov_base = &endmsg;
			recv_list[0].iov_len = sizeof(unsigned int);
			data_hdr.msg_name = 0;
			data_hdr.msg_namelen = 0;
			data_hdr.msg_iov = recv_list;
			data_hdr.msg_iovlen = 1;
			data_hdr.msg_control = 0;
			data_hdr.msg_controllen = 0;

			nread = ::recvmsg(m_tcpSockFd, &data_hdr, 0);

			if (nread < 0)
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
						nbytes =  totread;
						return SON_PROTOCOL::FAILURE;
						break;
				}
			}


			// ----------------------------------------------------------------
			//	We may have received End of Transmission, however, there may be
			//	packets lost that may not be accounted for...
			// 	This list is from the max_recvd_packet to last_packet
			//	We update the loss_list if needed
			// ----------------------------------------------------------------

			if (0 == endmsg)
			{
				// We may have lost packets and received an End message....
				if (max_recvd_packet != last_packet)
				{
					cout << " END Packet Loss " << max_recvd_packet + 1  <<
							" TO : " << last_packet <<  endl;

					// Increment the total number of lost packets
					lost_packets += last_packet -  max_recvd_packet  ;

					for (unsigned int i = (max_recvd_packet + 1) ; i <= last_packet; i++)
					{
						error_list[i/32].set( i % 32, 1);
					}
				}

				end_of_transmission =  true;
				cout << " Received TCP ENd of Transmission " <<  endl;
			}
		}
	}

	nbytes = totread;
	cout << "  Total Lost packets = " << lost_packets <<  endl;
	return SON_PROTOCOL::SUCCESS;

}



} // end of namespace
