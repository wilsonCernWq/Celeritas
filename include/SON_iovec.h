/******************************************************************************
 * Copyright(C) 2006 Venkatram Vishwanath Electronic Visualization Laboratory,  
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
 * Direct questions, comments etc about SYNOPTIC to venkat@evl.uic.edu
 *****************************************************************************/

#ifndef __SON__IOVEC__H
#define __SON__IOVEC__H

// Include iostream
#ifndef __IOSTREAM
#define __IOSTREAM
#include <iostream>
#endif

// Include  unistd.h
#ifndef __UNISTD__H
#define __UNISTD__H
#include <unistd.h>
#endif

//Include <sys/uio.h>
#ifndef __SYS__UIO__H
#define __SYS__UIO__H
#include <sys/uio.h>
#endif

//Include <sys/types.h>
#ifndef __SYS__TYPES__H
#define __SYS__TYPES__H
#include <sys/types.h>
#endif


#ifndef __SON__COMMON__H
#include "SON_common.h"
#endif


/** Note  : IOV_MAX value
*	The max value of an IOVEC array is given by IOV_MAX and is generally 1024
*	This is true for linux and Apple
*/
#ifndef IOV_MAX
#define IOV_MAX 1024
#endif


using namespace std;

namespace SON_PROTOCOL
{

/*! \class SON_iovec Scatter Gather Buffer class.
*	\brief A class used for scatter gather optimization for Data Trnsfer.
*/
class SON_iovec
{
	public:
		
		SON_iovec(int size);
			/*!< Constructor which takes total buffers to be used as a hint */
	 
		SON_iovec();
			/*!< Default Constructor */
		
		virtual ~SON_iovec();
			/*!< Destructor */

		int add(void* buff, long long int length);
			/*!< Add a buffer of the given Length */			
		
		int clear(void);
			/*!< Clear all added buffer and set the UsedBuffers to 0 */
		
		int getUsedBuffers(void);
			/*!< Return the number of used buffers */
				
		struct iovec* getIovecPtr(int index);
				
		void* getIovecBufferPtr(int index);
		
		void* getIovecBufferPtr(int index, long long int offset);
			
		long long int getIovecBufferLen(int index);
					
		int getAllocBuffers(void);
			/*!< Return the number of Allocated Buffers */
	
		long long int getTotalLen(void);
	
	
	protected:
		int m_numAllocated;
			/*!< The number of allocated buffers */
		
		int m_numUsed;
			/*!< The numbers of buffers i.e iovec structures needed/used */

		long long int m_totLen;
			/*!< The Total Memory Allocated in the IOVEC object */

		struct iovec* m_iovecList;
			/*!< The Buffer List */

};

}

#endif
