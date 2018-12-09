/** @file    AnalyzeDataNewInterface.h
  * @note    HANGZHOU Hikvision Software Co.,Ltd.All Right Reserved.
  * @brief   
  * 
  * @author  PlaySDK
  * @date    25/4/2012  
  * 
  * @note    version 4.0.0.8
  *
  * @warning  
  */

#ifndef _ANALYZEDATA_NEW_INTERFACE_H_
#define _ANALYZEDATA_NEW_INTERFACE_H_

#include "AnalyzeDataDefine.h"

#ifdef WIN32
	#if defined(ANALYZE_DADA_DLL)
		#define ANALYZEDATA_API  __declspec(dllexport) 
	#else 
		#define ANALYZEDATA_API  __declspec(dllimport) 
	#endif
#else
	#ifndef __stdcall
	   #define __stdcall
	#endif

	#ifndef ANALYZEDATA_API
	   #define  ANALYZEDATA_API
	#endif
#endif


#ifdef __cplusplus		
extern "C" 
{
#endif

/**	@fn	ANALYZEDATA_API void * __stdcall HIKANA_CreateStreamEx(unsigned long dwSize, unsigned char * pHeader)
 *	@brief	<create new analyze handle>
 *	@param  dwSize  [IN] wanted size for internal buffer
 *	@param  pHeader [IN] header buffer
 *	@return new analyze handle
 */
ANALYZEDATA_API void * __stdcall HIKANA_CreateStreamEx(unsigned long dwSize, unsigned char * pHeader);

/**	@fn	ANALYZEDATA_API void   __stdcall HIKANA_Destroy(void * hHandle)
 *	@brief	<destroy new analyze handle>
 *	@param  hHandle  [IN] new analyze handle
 *	@return none
 */	
ANALYZEDATA_API void   __stdcall HIKANA_Destroy(void * hHandle);

/**	@fn	ANALYZEDATA_API int   __stdcall HIKANA_InputData(void * hHandle, unsigned char * pBuffer, unsigned long dwSize)
 *	@brief	<input data to new analyze handle>
 *	@param  hHandle  [IN] new analyze handle
 *	@param  pBuffer  [IN] data buffer
 *	@param  dwSize   [IN] size of data
 *	@return TRUE if sucess, otherwise FALSE
 */	
ANALYZEDATA_API int   __stdcall HIKANA_InputData(void * hHandle, unsigned char * pBuffer, unsigned long dwSize);

/**	@fn	ANALYZEDATA_API int	   __stdcall HIKANA_GetOnePacketEx(void * hHandle, PACKET_INFO_EX* pstPacket)
 *	@brief	<get one packet data>
 *	@param  hHandle   [IN] new analyze handle
 *	@param  pstPacket [OUT] pointer to packet
 *	@return ERROR_NO on success, otherwise an error code is returned
 */	
ANALYZEDATA_API int	   __stdcall HIKANA_GetOnePacketEx(void * hHandle, PACKET_INFO_EX* pstPacket);

/**	@fn	ANALYZEDATA_API int   __stdcall HIKANA_ClearBuffer(void * hHandle)
 *	@brief	<clear buffer>
 *	@param  hHandle   [IN] new analyze handle
 *	@return TRUE on success, otherwise FALSE
 */
ANALYZEDATA_API int   __stdcall HIKANA_ClearBuffer(void * hHandle);

/**	@fn	ANALYZEDATA_API int	__stdcall HIKANA_GetRemainData(void * hHandle, unsigned char* pData, unsigned long* pdwSize)
 *	@brief	<get remain data>
 *	@param  hHandle   [IN] new analyze handle
 *	@param  pData     [IN] pointer to data bufffer
 *	@param  pdwSize   [OUT] pointer to data size
 *	@return ERROR_NO on success, otherwise an error code is returned
 */
ANALYZEDATA_API int	   __stdcall HIKANA_GetRemainData(void * hHandle, unsigned char* pData, unsigned long* pdwSize);

/**	@fn	ANALYZEDATA_API unsigned int  __stdcall HIKANA_GetLastErrorH(void * hHandle)
 *	@brief	<get remain data>
 *	@param  hHandle   [IN] new analyze handle
 *	@return last error
 */
ANALYZEDATA_API unsigned int  __stdcall HIKANA_GetLastErrorH(void * hHandle);

#ifdef __cplusplus 
	}	
#endif	

#endif//end _ANALYZEDATA_NEW_INTERFACE_H_