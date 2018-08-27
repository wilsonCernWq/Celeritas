/*
 *  testTCPplugin.cpp
 *  
 *
 *  Created by Venkatram Vishwanath on 5/7/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "LRAM_Net_Plugin.h"
#include "LRAM_Net_Client.h"

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
	if (argc != 4)
	{
		cout << " <exec> <Library Path> <IP Addr of Server > <PORT> " << endl;
		exit(-1);
	}

	mylib = new LRAM_Net_Plugin(argv[1]);
   
	if (mylib == 0 )
        cout << "  Error opening the lib" << endl;
    else
        cout << " Successfully Opened the Library" << endl;

	LRAM_Net_Client* obj = 0;
	int status;
	
	obj = (LRAM_Net_Client* ) mylib->getClientObj();
	if (obj ==0)
        cout << "Error creating Server obj" << endl;

	status = obj->connectToServer(argv[2], atoi(argv[3]));
	if (-1 == status)
		cout << " Error Connecting to Server" << endl;
	else
		cout << " Successfully Connected to Server " << endl;

	cout << obj->getConnectionInfo() << endl;
	
	char buf[4096];
	long long buflen = 4096;
	
	status = obj->write(buf, buflen);
	if ( -1 == status)
		cout << " Error Writing the Data " << endl; 
	else
		cout << " Client Successfully wrote Data" << endl;

	buflen = 4096;
	status = obj->read(buf, buflen);
	if ( -1 == status)
		cout << " Error Reading the Data " << endl; 
	else
		cout << " Client Successfully Read the Data" << endl;

	obj->close();
	delete obj;
	obj = 0;
	
	delete mylib;
	mylib = 0;

}
