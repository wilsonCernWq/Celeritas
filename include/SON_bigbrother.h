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

#ifndef __SON__BB__H
#define __SON__BB__H

#include <signal.h>
#include <fcntl.h>

#ifndef __SON__COMMON__H
#include "SON_common.h"
#endif


namespace SON_PROTOCOL
{

void serverCleanup(int);


class SON_BigBrother
{

	public:
		
		SON_BigBrother();
		
		virtual ~SON_BigBrother();
				
		void printInterfaceList(void);
		
		string getInterfaceIP(string interface_name);
	
		vector <string> getInterfaceNameList ();

    vector <string> getInterfaceIPList ();
	
		static SON_BigBrother* getObj(void);
		
	
	protected:

		int __initInterfaceDB(void);


	protected:
	
		struct sigaction m_sigaction;
		
		map < string, string > m_interfaceDB;
		/*!< Device name, IP Address */

		static SON_BigBrother* m_bigbrother;

};
		

	void printInterfaceList(void);
	
	string getInterfaceIP(string interface_name);

}
#endif


