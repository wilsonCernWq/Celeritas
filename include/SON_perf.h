/*
 *  SON_perf.h
 *
 *
 *  Created by Venkatram Vishwanath on 12/25/07.
 *  Copyright 2007 EVL. All rights reserved.
 *
 */

#ifndef __SON__PERF__H
#define __SON__PERF__H

#ifndef __SON__COMMON__H
#include "SON_common.h"
#endif


namespace SON_PROTOCOL
{

/*! \class SON_perf
*	This class maintains performance statistics
*	of the connection, including,  Input BW, Output BW, Total Data Read
*	and Total Data Sent
*/
class SON_perf
{
	public:

		/*! Default Constructor */
		SON_perf();


		/*! Default Destructor */
		virtual ~SON_perf();


		/*! \brief Returns the Total Input/Read Bandwidth of the Connection
		*
		*	\return A double representing the Input/Read Bandwidth in
		*	in <B> bytes per sec (bps) </B>
		*/
		double getReadBW(void);


		double getReadBW_Mbps(void);


		/*! \brief Returns the Total Output/Write Bandwidth of the Connection
		*
		*	\return A double representing the Output/Write Bandwidth
		*	in <B> bytes per sec (bps) </B>
		*/
		double getWriteBW(void);


		double getWriteBW_Mbps(void);

		/*!  \brief Returns the Total Bandwidth of the Connection
		*	This returns the sum of the Input and Output Bandwidth.
		*
		*	\return  A double representing the sum of the Input
		*	and the Output Bandwidth in <B> bytes per sec (bps) </B>
		*	\sa getReadBW(void) and \sa getWriteBW(void)
		*/
		double getTotalBW(void);

		double getTotalBW_Mbps(void);


		/*!	\brief Returns the Amount of Data Read/Received
		*
		*	This function return the amount of data read/received by
		*	the connection.
		*	\return A double representing the amount of data read.
		*/
		double getDataRead(void);


		/*!	\brief Returns the Amount of Data Sent/Written
		*
		*	This function return the amount of data sent/written by
		*	the connection.
		*	\return A double representing the amount of data sent.
		*/
		double getDataWritten(void);


		/*!	\brief Update the Amount of Data Read/Received
		*
		*	This function updates the amount of data read/received by
		*	the connection.
		*	\param val [IN] A double representing the amount of data
		*	read in \b bytes.
		*/
		void updateDataRead(double val);


		/*!	\brief Update the Amount of Data Sent/Written
		*
		*	This function updates the amount of data sent/written by
		*	the connection.
		*	\param val [IN]  A double representing the amount of data sent
		*	in \b bytes.
		*/
		void updateDataWritten(double val);


		/*!	\brief Return the time the Connection was created
		*
		*	This function returns the time the Connection was created.
		*	This is used for calculation the Bandwidth.
		*	\return A timeval struct representing the starting time.
		*/
		struct timeval getConnStartTime(void);


		/*!	\brief Initializes the Start time of the Connection
		*
		*	This function sets the Start time of the Connection.
		*	This is used for calculation the Bandwidth.
		*	\warning This function is \b NOT Thread-Safe.
		*	and should not be used in user programs unless a necessity.
		*/
		void initConnStartTime(void);


	protected:

		double m_totRead;
			/*!< The total amount of Data read */


		pthread_rwlock_t* m_protectReadVal;
			/*!< A mutex to protect the amount of Data read */


		double m_totWrite;
			/*!< The total amount of Data written */


		pthread_rwlock_t* m_protectWriteVal;
			/*!< A mutex to protect the amount of Data written */


		struct timeval m_startTime;
			/*!< The base time with respect to which the BW
			*	calculations are done. This is set to the time
			*	the connection is created
			*/




};

} // End of Namespace


#endif


