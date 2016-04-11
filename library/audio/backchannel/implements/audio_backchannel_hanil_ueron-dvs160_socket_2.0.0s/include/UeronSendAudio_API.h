#ifndef __UERON_SENDAUDIO_API_H
#define __UERON_SENDAUDIO_API_H


////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EXPORTS
 #ifdef __cplusplus
	#define EXPORTS extern "C"
 #else
	#define EXPORTS
 #endif
#endif	// EXPORTS

#ifdef _WIN32
  #define WINAPI __stdcall
#else
  #define WINAPI
#endif	// _WIN32

/*------------------------------------------------------------------------------------------*/

/* Ueron SendAudio Max Client */
#define		UERON_SENDAUDIO_MAX_CLIENT			16

/* Ueron SendAudio Return Code */
#define		UERON_SUCCESS						1
#define		UERON_FAILURE						-1
#define		UERON_ERR_PARAM						-101		// parameter error
#define		UERON_ERR_ALLOCATION				-102		// failed to allocate memory
#define		UERON_ERR_CREATE					-103		// failed to create
#define		UERON_ERR_ALREADY_CREATED			-104		// already created
#define		UERON_ERR_CREATED    		     	-105		// not created
#define		UERON_ERR_INIT						-106		// not initiaized	
#define		UERON_ERR_NET_IP					-201
#define		UERON_ERR_NET_LOGON					-202
#define		UERON_ERR_NET_MAXCLIENT				-203
#define		UERON_ERR_NET_TIMEOUT				-204
#define		UERON_ERR_NET_INUSE					-205
#define		UERON_ERR_NET_CONNECTION			-206
#define		UERON_ERR_NET_RECEPTION				-207
#define		UERON_ERR_NET_TRANSMISSION			-208
#define		UERON_ERR_NET_SERIAL				-209
#define		UERON_ERR_NET_PTZ					-210
#define		UERON_ERR_NET_RELAY					-211
#define		UERON_ERR_NET_SENSOR				-212
#define		UERON_ERR_NET_REBOOT				-213
#define		UERON_ERR_AUD_ENC_TYPE				-401		// invalid encoder type
#define		UERON_ERR_AUD_ENC_FRAMESIZE			-402		// invalid encoder FrameSize
#define		UERON_ERR_AUD_DEC_TYPE				-403		// invalid decoder type
#define		UERON_ERR_AUD_DEC_FRAMESIZE			-404		// invalid decoder FrameSize
#define		UERON_ERR_AUD_DEC_FRAMETYPE			-405		// invalid frame of compress data 
#define		UERON_ERR_AUD_COD_DEVICE			-406		// invalid device handle
#define		UERON_ERR_AUD_COD_BUF				-407		// invalid data addr
#define		UERON_ERR_AUD_FILE					-408		// invalid wave file

/*------------------------------------------------------------------------------------------*/

/* Ueron Version struct */
#ifndef _UERON_VERSION_STRUCTURE_
#define _UERON_VERSION_STRUCTURE_
#pragma pack(push, 2)
typedef struct UeronVersion
{
	unsigned int	major;
	unsigned int	minor;
	unsigned int	build;
	unsigned int	revision;
} UeronVersion, *pUeronVersion;
#pragma pack(pop)
#endif	// _UERON_VERSION_STRUCTURE_

/*------------------------------------------------------------------------------------------*/

/******************************************************************************
* Function:      UeronSendAudio_GetVersion
* Description:   Get version information 
* Input:         none 
* Output:        pVersion                  version describe struct
* Return:        int                       UERON_SUCCESS / UERON_FAILURE
******************************************************************************/
int WINAPI	UeronSendAudio_GetVersion(UeronVersion *pVersion);

/***************************************************************************
* Function:      UeronSendAudio_Create
* Description:   Create SendAudio
* Input:         none
* Output:        *pId                       ID
* Return:        UERON_SUCCESS              success
*                UERON_ERR_CREATE           fail to create
******************************************************************************/
int WINAPI			UeronSendAudio_Create(int *pId);

/***************************************************************************
* Function:      UeronSendAudio_CreateEx
* Description:   Create SendAudio
* Input:         nId                        ID
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_ALREADY_CREATED  already created
*                UERON_ERR_CREATE           fail to create
******************************************************************************/
int WINAPI			UeronSendAudio_CreateEx(int nId);

/***************************************************************************
* Function:      UeronSendAudio_Destroy
* Description:   Destroy SendAudio
* Input:         nId                        ID
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
******************************************************************************/
int WINAPI			UeronSendAudio_Destroy(int nId);

/***************************************************************************
* Function:      UeronSendAudio_IsCreate
* Description:   Check SendAudio
* Input:         nId                        ID
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
******************************************************************************/
int WINAPI			UeronSendAudio_IsCreate(int nId);

/***************************************************************************
* Function:      UeronSendAudio_SetConnectionInfo
* Description:   Set connection info
* Input:         nId                        ID
                 *pAddress                  server ip
				 nPort                      server port
				 *pUserid                   user id
				 *pPassword                 user password
				 nTimeout                   socket timeout
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
******************************************************************************/
int WINAPI			UeronSendAudio_SetConnectionInfo(
											int nId,
											char *pAddress,
											unsigned int nPort,
											char *pUserid,
											char *pPassword,
											unsigned int nTimeout);

/***************************************************************************
* Function:      UeronSendAudio_ConnectServer
* Description:   Connect server
* Input:         nId                        ID
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
*                UERON_ERR_NET_INUSE        already connected
*                UERON_ERR_NET_CONNECTION   failed to connect
******************************************************************************/
int WINAPI			UeronSendAudio_ConnectServer(int nId);

/***************************************************************************
* Function:      UeronSendAudio_DisconnectServer
* Description:   Disconnect server
* Input:         nId                        ID
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
******************************************************************************/
int WINAPI			UeronSendAudio_DisconnectServer(int nId);

/***************************************************************************
* Function:      UeronSendAudio_SendStreamBuffer
* Description:   Send audio stream data
* Input:         nId                        ID
				 nLength                    stream data length
				 *pBuffer                   stream data
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
*                UERON_ERR_NET_INUSE        already connected
*                UERON_ERR_NET_CONNECTION   failed to connect
*                UERON_ERR_NET_TRANSMISSION failed to transmit
******************************************************************************/
int WINAPI			UeronSendAudio_SendStreamBuffer(
											int nId,
											unsigned int nLength,
											unsigned char *pBuffer);

/***************************************************************************
* Function:      UeronSendAudio_SendMediaFile
* Description:   Send data of audio file
* Input:         nId                        ID
                 *pFile                     file name
* Output:        none
* Return:        UERON_SUCCESS              success
*                UERON_ERR_PARAM		    invalid parameter
*                UERON_ERR_CREATED          not created
*                UERON_ERR_NET_INUSE        already connected
*                UERON_ERR_NET_CONNECTION   failed to connect
*                UERON_ERR_NET_TRANSMISSION failed to transmit
******************************************************************************/
int WINAPI			UeronSendAudio_SendMediaFile(
											int nId,
											char *pFile);


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif	/* __UERON_SENDAUDIO_API_H */
