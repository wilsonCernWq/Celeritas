#include "LRAM_Net_Plugin.h"
#include "LRAM_Util_PrintMessage.h"


LRAM_Net_Plugin :: LRAM_Net_Plugin (const char* libpath)
{	
	char *errStr = NULL;
	struct stat statBuf;
			
	/* stat the file to see if it actually exists first*/
	if (0 != ::stat(libpath, &statBuf))
	{
		LRAM_PRINT_PERROR( \
			"LRAM_Net_Plugin::CTR Error stat'ing %s \n", libpath);
			
		m_pluginHandle = NULL;
		return ;
	}

	m_pluginHandle = ::dlopen(libpath, RTLD_LAZY);

	errStr = ::dlerror();
	if (errStr)
	{
		LRAM_PRINT_PERROR ( \
			"LRAM_Net_Plugin :: CTR %s : While loading up : %s \n", \
			errStr, libpath );
								
		m_pluginHandle = NULL;
	}

}// End of LRAM_Plugin_Dlib()




void* LRAM_Net_Plugin :: getClientObj(void)
{
	char *errStr = NULL; 
		/* error String if any */
		
	void (*tempFunc)(void *);
		/* Pointer to the actual function */
		
	void *objPtr;
		/* Pointer to the object which is returned back */

	if (0 == m_pluginHandle)
		return 0;
		
	/* get the handle for the function which then would return the
	* object of the plugin.
	*/
	tempFunc = (void (*)(void*))dlsym(m_pluginHandle, "getClientObj");
	
	if( (errStr = ::dlerror()) )
	{
		LRAM_PRINT_ERROR("LRAM_Net_Plugin::getClientObj(): %s \n", errStr);
		return 0;
	}
	
	/* Now call the function which will return a pointer to the object */
	tempFunc(&objPtr);
	
	return objPtr;
	
} //End of getClientObj()



void* LRAM_Net_Plugin :: getServerObj(void)
{
	char *errStr = NULL; 
		/* error String if any */
		
	void (*tempFunc)(void *);
		/* Pointer to the actual function */
		
	void *objPtr;
		/* Pointer to the object which is returned back */

	if (0 == m_pluginHandle)
		return 0;
		
	/* get the handle for the function which then would return the
	* object of the plugin.
	*/
	tempFunc = (void (*)(void*))dlsym(m_pluginHandle, "getServerObj");
	
	if( (errStr = ::dlerror()) )
	{
		LRAM_PRINT_ERROR("LRAM_Net_Plugin::getServerObj(): %s \n", errStr);
		return 0;
	}
	
	/* Now call the function which will return a pointer to the object */
	tempFunc(&objPtr);
	
	return objPtr;
	
} //End of getServerObj()




int LRAM_Net_Plugin :: delClientObj (void *objPtr)
{
	char *errStr = NULL;
	void (*tempFunc)(void *);

	if (0 == m_pluginHandle)
		return -1;
		
	/** get the handle for the function which then would return the 
	* object of the plugin.
	*/
	tempFunc = (void (*)(void*))dlsym(m_pluginHandle, "delClientObj");
	if( (errStr = dlerror()) )
	{
		LRAM_PRINT_ERROR("LRAM_Net_Plugin :: delClientObj(): %s \n", errStr);
		return -1;
	}

	tempFunc(&objPtr);
	
	return 0;

} //End of delClientObj()



int LRAM_Net_Plugin :: delServerObj (void *objPtr)
{
	char *errStr = NULL;
	void (*tempFunc)(void *);

	if (0 == m_pluginHandle)
		return -1;
		
	/** get the handle for the function which then would return the 
	* object of the plugin.
	*/
	tempFunc = (void (*)(void*))dlsym(m_pluginHandle, "delServerObj");
	if( (errStr = dlerror()) )
	{
		LRAM_PRINT_ERROR("LRAM_Net_Plugin :: delServerObj(): %s \n", errStr);
		return -1;
	}

	tempFunc(&objPtr);
	
	return 0;

} //End of delServerObj()



LRAM_Net_Plugin :: ~LRAM_Net_Plugin()
{
	/* unload the library */
	if (m_pluginHandle == NULL)
		return;

	::dlclose(m_pluginHandle);

} //End of LRAM_Net_Plugin()

