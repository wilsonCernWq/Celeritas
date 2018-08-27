/*
 *  testTCPplugin.cpp
 *  
 *
 *  Created by Venkatram Vishwanath on 5/7/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "LRAM_Net_Plugin.h"
#include "LRAM_Net_Server.h"

#include <iostream>

using namespace std;

int main (int argc, char** argv)
{
	LRAM_Net_Plugin* mylib = 0;
	
	/*	
	#ifdef __APPLE__
    mylib = new LRAM_Net_Plugin("./libtcp.dylib");
    #else
    mylib = new LRAM_Net_Plugin("./libtcp.so");
    #endif
    */

    if (argc != 3)
    {
        cout << " <exec> <Library Path> <PORT > " << endl;
        exit(-1);
    }

    mylib = new LRAM_Net_Plugin(argv[1]);
	if (mylib == 0 )
        cout << "  Error opening the lib" << endl;
    else
        cout << " Successfully Opened the Library" << endl;

	LRAM_Net_Server* obj = 0;
	
	obj = (LRAM_Net_Server* ) mylib->getServerObj();
	if (obj ==0)
        cout << "Error creating Server obj" << endl;

	int status = obj->init(atoi(argv[2]));
	cout << " Initialized the Server " << endl;
	
	LRAM_Net_Client* cliobj = 0;

	cout << " Waiting for a Client to Connect " << endl;
	cliobj = obj->waitForNewConnection();

	if (0 == cliobj)
		cout << " Error Creating a Client" << endl;

	cout << cliobj->getConnectionInfo() << endl;
	
	char buf[4096];
	long long buflen = 4096;
	
	buflen = 4096;
	status = cliobj->read(buf, buflen);
	if ( -1 == status)
		cout << " Error Reading the Data " << endl; 
	else
		cout << " Client Successfully Read the Data" << endl;
	
	status = cliobj->write(buf, buflen);
	if ( -1 == status)
		cout << " Error Writing the Data " << endl; 
	else
		cout << " Client Successfully wrote Data" << endl;

	cliobj->close();
	
	obj->close();
	delete obj;
	obj = 0;
	
	delete mylib;
	mylib = 0;

}
