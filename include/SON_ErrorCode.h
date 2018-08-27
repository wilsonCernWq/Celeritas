/*
 *   MultiRAIL: 
 *  
 *  Copyright (C) 2007-2008   Electronic Visualization Laboratory (EVL),
 *	Venkatram Vishwanath, Jason Leigh, University of Illinois at Chicago (UIC)
 *	All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:		Venkatram Vishwanath
 *	Email:		venkat@evl.uic.edu
 *
 *	Version:
 *
 *	History:
 *	
*/
 

#ifndef __SON__ERRORCODE__H
#define __SON__ERRORCODE__H


namespace SON_PROTOCOL 
{
	static const int SUCCESS = 0;
		/*!< Successful Operation */
		
	static const int FAILURE = -1;
		/*!< An Error Occured */

	static const int TIME_OUT = -2;
		/*!< A Time Out  Occured */
} // End of Namesopace

#endif


