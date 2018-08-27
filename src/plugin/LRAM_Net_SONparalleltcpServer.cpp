#include "LRAM_Net_SONparalleltcpServer.h"
#include "LRAM_Net_SONparalleltcpClient.h"


LRAM_Net_SONparalleltcpServer :: LRAM_Net_SONparalleltcpServer()
{
	m_server = new SON_paralleltcpServer();
}

LRAM_Net_SONparalleltcpServer :: ~LRAM_Net_SONparalleltcpServer()
{
	if (0 != m_server)
		delete m_server;
	
	m_server = 0;	
}


int LRAM_Net_SONparalleltcpServer :: init(int port)
{	
	return m_server->init(port);
}


int LRAM_Net_SONparalleltcpServer :: setProtocolOptions( int option, int value)
{
	return 0;
}


LRAM_Net_Client* LRAM_Net_SONparalleltcpServer :: waitForNewConnection()
{
	SON_paralleltcpClient  *ntwkclient = 0;
	ntwkclient = m_server->waitForNewConnection();

	/* An error occured */
	if (0 == ntwkclient)
		return 0;
	
	LRAM_Net_SONparalleltcpClient* client = 
								new LRAM_Net_SONparalleltcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


LRAM_Net_Client* LRAM_Net_SONparalleltcpServer :: checkForNewConnection
												(int timeoutinsecs)
{
	SON_paralleltcpClient  *ntwkclient = 0;
	ntwkclient = m_server->checkForNewConnection(timeoutinsecs);

	/* Error or a Timeout */	
	if ( 0 == ntwkclient)
		return 0;

	LRAM_Net_SONparalleltcpClient* client = 
							new LRAM_Net_SONparalleltcpClient(ntwkclient);
	
	return  (LRAM_Net_Client*) client;
}


int LRAM_Net_SONparalleltcpServer :: close()
{
	return m_server->close();
}
