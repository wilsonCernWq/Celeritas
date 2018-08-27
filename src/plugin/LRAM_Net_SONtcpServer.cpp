#include "LRAM_Net_SONtcpServer.h"
#include "LRAM_Net_SONtcpClient.h"


LRAM_Net_SONtcpServer :: LRAM_Net_SONtcpServer()
{
	m_server = new SON_tcpServer();
}

LRAM_Net_SONtcpServer :: ~LRAM_Net_SONtcpServer()
{
	if (0 != m_server)
		delete m_server;
	
	m_server = 0;	
}


int LRAM_Net_SONtcpServer :: init(int port)
{	
	return m_server->init(port);
}


int LRAM_Net_SONtcpServer :: setProtocolOptions( int option, int value)
{
	return 0;
}


LRAM_Net_Client* LRAM_Net_SONtcpServer::waitForNewConnection()
{
	SON_tcpClient  *ntwkclient = 0;
	ntwkclient = m_server->waitForNewConnection();
	
	LRAM_Net_SONtcpClient* client = new LRAM_Net_SONtcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


LRAM_Net_Client* LRAM_Net_SONtcpServer :: checkForNewConnection
											(int timeoutinsecs)
{
	SON_tcpClient  *ntwkclient = 0;
	ntwkclient = m_server->checkForNewConnection(timeoutinsecs);
	
	if ( 0 != ntwkclient)
	{
		LRAM_Net_SONtcpClient* client = new LRAM_Net_SONtcpClient(ntwkclient);
	
		return  (LRAM_Net_Client*) client;
	}
	 
	return 0;
}


int LRAM_Net_SONtcpServer :: close()
{
	return m_server->close();
}
