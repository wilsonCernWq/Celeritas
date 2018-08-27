#ifndef __LRAM__NET__PLUGIN__H
#define __LRAM__NET__PLUGIN__H

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>


extern "C"
{
	void getClientHandle(void *&);
		/** This is a free floating C function which will be present within each DL
		* 	and when called, it will create and return an
		* 	object of the plugin int the parameter passed to it.
		*/
     
	void getServerHandle(void *&);						   
	
	
	void delClientObj(void *&);
		/* Delete the object whose handle is passed */


	void delServerObj(void *&);

}


/*! \class LRAM_Net_Plugin
*	
*	\brief A generic network plugin class for reliable communication
*
*/

class LRAM_Net_Plugin
{
	public:
	
		LRAM_Net_Plugin (const char* libpath);  
			/* loads up the dynamic lib into the process space.*/
		
		
		/*! Returns an object of the class in the DLL.
		* 	It returns null if it cannot get the symbol
		*/
		void* getClientObj(void);
		
			
		void* getServerObj(void);

		/*! \brief Call the Destructor of the handle.
		*
		*	A plugin is expected to implement a function that would 
		*			call its destructor.
		*
		*	@Input: 	void* The handle(Object) of the Library.
		*
		*	\return	0 on Success \n
		*			-1 on error if the handle is NULL
		*/

		int delClientObj(void *);
		
					
		int delServerObj(void *);						
		
		virtual ~LRAM_Net_Plugin();
			/* automatically unloads the DL */
			
	protected:
	
		void* m_pluginHandle;
			/*!< The library handle */ 
};


#endif

