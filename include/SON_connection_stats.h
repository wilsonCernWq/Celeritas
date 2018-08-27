#ifndef __SYNOPTIC__CONNECTION__PROPERTY__H
#define __SYNOPTIC__CONNECTION__PROPERTY__H
/** A base level socket class subclassed by QUANTAnet_tcp_c, QUANTAnet_udp_c, QUANTAnet_mcast_c, QUANTAnet_parallelTcp_c,  and QUANTAnet_perfMonitor_c. The class itself does no networking. It provides some common member functions that are useful for network coding. In general one does not create an instance of this type.

    This class also offers information about the bandwidth, latency, Inter-Message_delay, Jitter and Burstiness values associated with the individual socket connections. Latency and Jitter calculations are offered only if the individual subclasses have implemented and enabled it. Otherwise values will be zero.
 These calculations are used by the QUANTAnet_perfMonitor_c class
*/

class SON_ConnectionProperty {

	public:

		SON_ConnectionProperty();
	
		virtual ~SON_ConnectionProperty() {}

		int calculateMaxBufferSize( void );
		
		int calculateDefaultBufferSize( void );

	protected:
	
		int m_mtuSize;
	
		double m_rttval;
	
		double m_totalDataSent;
		
		double m_totalDataRecvd;
		
		double m_startTime;
		
		
		/* Common for TCP and UDP */
		int m_readBufferSizeDefault;

		int m_readBufferSizeMax;

		int m_readBufferSizeVal;
		
		int m_writeBufferSizeDefault;
		
		int m_writeBufferSizeMax;
		
		int m_writeBufferSizeVal;
		
		
		

		// Need values  for the following
		
		/*
		1. jitter
		2. Instantaneous BW
		3. Burstiness
		4. Shot Term Averages
		5. ethernet device that the socket is bound to.						
		*/
		
};


#endif
