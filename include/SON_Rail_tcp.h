#include "SON_tch.h"

namespace  SON_PROTOCOL
{


class SON_Rail_tcpClient;

class SON_Rail_tcpServer : public SON_tcpServer
{
	public:

		SON_Rail_tcpServer();

		virtual ~SON_Rail_tcpServer();

	    virtual SON_Rail_tcpClient* waitForNewConnection();

        virtual SON_Rail_tcpClient* checkForNewConnection(int timeoutinsecs);

		virtual string getInterface(void);

		virtual int getAffinity(MultiRail_Mask& mask);

		virtual int setAffinity(MultiRail_Mask& mask);

	protected:

		MultiRail_Interrupt_Affinity* m_intAffinity;

		string m_interface;
};

class SON_Rail_tcpClient : public SON_tcpClient
{
	public:
		SON_Rail_tcpClient();

		virtual ~SON_Rail_tcpClient();

		virtual string getInterface(void);

		virtual int getAffinity(MultiRail_Mask& mask);

		virtual int setAffinity(MultiRail_Mask& mask);

		virtual int setAffinityToPhysicalAffinity(void);		

	protected:

		MultiRail_Interrupt_Affinity* m_intAffinity;
		
		string m_interface;
};

} // End of namespace
