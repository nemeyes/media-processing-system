/**
 * NVSNetLib.h
 * 
 */
#ifndef _DDKFG_20050422_15033355_NVS_NETLIB__
#define _DDKFG_20050422_15033355_NVS_NETLIB__

#define NVSNETLIB_API EXTERN_C __declspec(dllimport)
/*
#ifdef WIN32
    #ifdef _USRDLL
        #define NVSNETLIB_API EXTERN_C __declspec(dllexport)
    #elif _LIB
        #define NVSNETLIB_API EXTERN_C
    #else
        #define NVSNETLIB_API EXTERN_C __declspec(dllimport)
    #endif
#else
    #define NVSNETLIB_API
#endif
*/


#ifndef MAKEFOURCC
    #ifdef _BIG_ENDIAN_
        #define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch3) | ((unsigned int)(unsigned char)(ch2) << 8) | ((unsigned int)(unsigned char)(ch1) << 16) | ((unsigned int)(unsigned char)(ch0) << 24 ))
    #else
        #define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
    #endif
#endif

#define NETSESSION_ERROR                                -1
#define NETSESSION_OK                                   0



#define NETSESSION_STAT_INIT                            0
#define NETSESSION_STAT_CONNECTING                      1
#define NETSESSION_STAT_SEND_LOGIN                      2
#define NETSESSION_STAT_WAIT_LOGIN                      3
#define NETSESSION_STAT_RECV                            4
#define NETSESSION_STAT_CLOSE                           5
#define NETSESSION_STAT_SLEEP                           6



#define NETSESSION_RET_NORMAL                           0x00000000      /*    Nothing happended in the library  */

#define NETSESSION_RET_LOGIN_OK_MASK                    0x0F000000      
#define NETSESSION_RET_LOGIN_SUCCESS                    0x01000000
#define NETSESSION_RET_GET_VIDEO                        0x02000000
#define NETSESSION_RET_GET_AUDIO                        0x03000000
#define NETSESSION_RET_PARTIAL_LOGIN_SUCCESS            0x04000000
#define NETSESSION_RET_TYPE_CHANGED                     0x05000000
#define NETSESSION_RET_GET_ACK                          0x06000000
#define NETSESSION_RET_GET_EVENT                        0x07000000

#define NETSESSION_RET_GET_EVENT_START                  0x08000000
#define NETSESSION_RET_GET_EVENT_STOP                   0x09000000

#define NETSESSION_RET_GET_CONTROL                      0x0A000000
// in this case the recvData member contains data as the next form.
#ifdef WIN32
#pragma warning(disable : 4200)
#endif
typedef struct SerialData{
#define FCC_SERIALDATA      MAKEFOURCC('S', 'E', 'R', 'I')
    unsigned int dwControlType;
#define RS_232          MAKEFOURCC('P', '2', '3', '2')
#define RS_485          MAKEFOURCC('P', '4', '8', '5')
    unsigned int    dwSerialType;       // P232 / P485
    unsigned int    dwDataSize;
    unsigned int    dwChannel;      // channel from which the data was
    unsigned char   bSerialData[256];
} SerialData, *pSerialData;

typedef struct MotionMapData{
#define FCC_MOTIONDATA      MAKEFOURCC('M', 'O', 'D', 'E')
    unsigned int    dwControlType;
    unsigned int    dwDataSize;
    unsigned short  motionData[256];
} MotionData, *pMotionData;

// for MR904
typedef struct DigitalInData{
#define FCC_DIGITALIN      MAKEFOURCC('D', 'I', 'N', 'D')
    unsigned int    dwControlType;
    unsigned int    dwDataSize;
    unsigned int	dwStatus;
} DigitalInData, *pDigitalInData;

#ifdef WIN32
#pragma warning(default : 4200)
#endif

#define NETSESSION_RET_ADNORMAL                         -1

#define NETSESSION_RET_LOGIN_FAIL_MASK                  0xF0000000
#define NETSESSION_RET_LOGIN_FAIL_MAX                   0x10000000
#define NETSESSION_RET_LOGIN_FAIL_AUTH                  0x20000000
#define NETSESSION_RET_CONNECT_ERROR                    0x30000000
#define NETSESSION_RET_CONNECTION_TIMEOUT               0x40000000
#define NETSESSION_RET_NETWORK_ERROR                    0x50000000

#define NETSESSION_RET_EVENT_MASK                       0x00FF0000
#define NETSESSION_RET_EVENT_MOTION1                    0x00010000
#define NETSESSION_RET_EVENT_MOTION2                    0x00020000
#define NETSESSION_RET_EVENT_MOTION3                    0x00040000
#define NETSESSION_RET_EVENT_MOTION4                    0x00080000
#define NETSESSION_RET_EVENT_DINPUT                     0x00100000
#define NETSESSION_RET_EVENT_VIDEOSIGNAL                0x00200000
#define NETSESSION_RET_EVENT_DOUTPUT                    0x00400000

#define NETSESSION_RET_VIDEO_CHANED_MASK                0x0000FF00
#define NETSESSION_RET_VIDEO_ENCODING_CHANGED           0x00000100
#define NETSESSION_RET_VIDEO_FRAMEMODE_CHANGED          0x00000200
#define NETSESSION_RET_VIDEO_VBR_CHANGED                0x00000400
#define NETSESSION_RET_VIDEO_BITRATE_CHANGED            0x00000800
#define NETSESSION_RET_VIDEO_WIDTH_CHANGED              0x00001000
#define NETSESSION_RET_VIDEO_HEIGHT_CHANGED             0x00002000
#define NETSESSION_RET_VIDEO_GOPSIZE_CHANGED            0x00004000
#define NETSESSION_RET_VIDEO_FPS_CHANGED                0x00008000

/*  The following flags are duplicated as videos'. */
#define NETSESSION_RET_AUDIO_CHANED_MASK                0x0000FF00
#define NETSESSION_RET_AUDIO_ENCODING_CHANGED           0x00000100
#define NETSESSION_RET_AUDIO_CHANNEL_CHANGED            0x00000200
#define NETSESSION_RET_AUDIO_BPS_CHANGED                0x00000400
#define NETSESSION_RET_AUDIO_SAMPLERATE_CHANGED         0x00000800

//return value for sendAudio
#define NETSESSION_RET_AU_INVALID_ARGUMENT              -1
#define NETSESSION_RET_AU_INVALID_SOCKET                -2
#define NETSESSION_RET_AU_AUDIO_NOT_AVAILABLE           -3
#define NETSESSION_RET_AU_PACKET_IS_TOO_LARGE           -4
#define NETSESSION_RET_AU_NETWORK_ERROR                 -5

//return value for sendControl
#define NETSESSION_RET_SC_INVALID_ARGUMENT              -1
#define NETSESSION_RET_SC_INVALID_SOCKET                -2
#define NETSESSION_RET_SC_QUEUE_FULL                    -3
#define NETSESSION_RET_SC_PACKET_IS_TOO_LARGE           -4
#define NETSESSION_RET_SC_NETWORK_ERROR                 -5

#define NETSESSION_LOGIN_TYPE_VIDEO                     0x01
#define NETSESSION_LOGIN_TYPE_AUDIO                     0x02
#define NETSESSION_LOGIN_TYPE_AUDIO_PLAYBACK            0x04
#define NETSESSION_LOGIN_TYPE_CONFIG                    0x08
#define NETSESSION_LOGIN_TYPE_EVENT                     0x10
#define NETSESSION_LOGIN_TYPE_EVENT_VIDEO               0x20

#if !defined(ARM_940T) && !defined(_SOLARIS_)
#pragma pack(1)
#endif

/*----------------------------------------------------------------------------*/
struct NSVideoInfo {
#define FCC_MP4S        MAKEFOURCC('M', 'P', '4', 'S')
#define FCC_DX50        MAKEFOURCC('D', 'X', '5', '0')
#define FCC_MJPG        MAKEFOURCC('M', 'J', 'P', 'G')
#define FCC_MPG2        MAKEFOURCC('M', 'P', 'G', '2')
#define FCC_MPG1        MAKEFOURCC('M', 'P', 'G', '1')
#define FCC_H263        MAKEFOURCC('H', '2', '6', '3')
#define FCC_H264        MAKEFOURCC('H', '2', '6', '4')
    unsigned int    encodingType;       // video compressor type.
#define VIFM_IONLY      0x01
#define VIFM_IPFRAME    0x02
#define VIFM_IPB        0x03
    unsigned char   frameMode;          // how to encode video stream.
    unsigned int    fps;                // 1 ~ 30
    unsigned char   vbr;                // if 0, use the bitrate member for const bitrate else over zero, various bitrate.
// Bitrate setting -- a unit : 1000 Kbps
#define BITRATE_8M          8000
#define BITRATE_7M          7000
#define BITRATE_6M          6000
#define BITRATE_5M          5000
#define BITRATE_4M          4000
#define BITRATE_2M          2000
#define BITRATE_1_5M        1500
#define BITRATE_1M          1000
#define BITRATE_750K        750
#define BITRATE_500K        500
#define BITRATE_384K        384
#define BITRATE_256K        256
#define BITRATE_128K        128
#define BITRATE_64K         64
#define BITRATE_32K         32
    unsigned int    bitrate;            // bitrate in kilo bit per second
    unsigned short  videoWidth;         // frame width
    unsigned short  videoHeight;        // frame height
#define VIVS_VSIGNAL        0x40
    unsigned char   videoStatus1;       // video signal
    unsigned char   videoStatus2;
    unsigned char   videoStatus3;
    unsigned char   videoStatus4;
#define VIVT_NTSC           0x00
#define VIVT_PAL            0x01
#define VIVT_SECAM          0x02
    unsigned char   videoType;          // video output type
    unsigned char   groupSize;          // group of picture size
#if defined(ARM_940T) || defined(_SOLARIS_)
} __attribute__((packed));
#else   
};
#endif

#if 0   // Hi3510
typedef enum hiAUDIO_CODEC_FORMAT_E
{
    AUDIO_CODEC_FORMAT_G711A   = 1,   /* G.711 A            */
    AUDIO_CODEC_FORMAT_G711Mu  = 2,   /* G.711 Mu           */
    AUDIO_CODEC_FORMAT_ADPCM   = 3,   /* IMA-ADPCM              */
    AUDIO_CODEC_FORMAT_G726    = 4,   /* G.726              */
    AUDIO_CODEC_FORMAT_AMR     = 5,   /* AMR encoder format */
    AUDIO_CODEC_FORMAT_AMRDTX  = 6,   /* AMR encoder formant and VAD1 enable */
    AUDIO_CODEC_FORMAT_AAC     = 7,   /* AAC encoder        */
    AUDIO_CODEC_FORMAT_BUTT
} AUDIO_CODEC_FORMAT_E;
#endif

/*----------------------------------------------------------------------------*/
struct NSAudioInfo {
#define FCC_IMAACPCM        MAKEFOURCC('I', 'P', 'C', 'M')
#define FCC_MSADPCM         MAKEFOURCC('M', 'P', 'C', 'M')
#define FCC_PCM             MAKEFOURCC('N', 'P', 'C', 'M')
#define FCC_G726            MAKEFOURCC('G', '7', '2', '6')
#define FCC_G711_A          MAKEFOURCC('A', '7', '1', '1')
#define FCC_G711_U          MAKEFOURCC('U', '7', '1', '1')
    unsigned int    encodingType;       // "IPCM" IMA ACPCM, "MPCM" MS ADPCM, "NPCM" PCM
#define AUDIO_MONO          0x01
#define AUDIO_STEREO        0x02
    unsigned char   channel;            // audio channel : Stereo(0x02), Mono(0x01)
    unsigned char   bitPerSample;       // 8 bit, 16 bit
    unsigned int    samplingRate;       // 8000, 11025, 16000, 22050, 44100, 48000
#if defined(ARM_940T) || defined(_SOLARIS_)
} __attribute__((packed));
#else   
};
#endif

/*----------------------------------------------------------------------------*/
struct NSServerInfo {
    unsigned char       cameraModel[4];     // "V20A"
    unsigned char       reserved;
    unsigned char       videoCount;         // video count
    unsigned char       audioCount;         // audio count 
    unsigned char       serverName[32];     // server name
    unsigned char       channelName[32];    // channel name
    unsigned char       channelNumber;      // channel number 1 ~ 4

    struct NSVideoInfo  videoInfo;
    struct NSAudioInfo  audioInfo;
#if defined(ARM_940T) || defined(_SOLARIS_)
} __attribute__((packed));
#else   
};
#endif

#if !defined(ARM_940T) && !defined(_SOLARIS_)
#pragma pack(4)
#endif

/*----------------------------------------------------------------------------*/
typedef struct CNetSession {
    int                 status;
    
    int                 fps;

    unsigned int        sequence;
    unsigned int        sec;
    unsigned int        usec;
    unsigned char       frameType;
    
    unsigned int        dataSize;
    char                *recvData;

    char                *winPosition;

    int                 isVideo;
    int                 isAudio;
    int                 isPlayBackAudio;
    int                 isConfig;
    
    int                 isEvent;
    int                 isEventVideo;
    
    struct NSServerInfo serverInfo;

    int                 (*close)(void *_this);
    int                 (*release)(void *_this);
    int                 (*sendAudio)(void *_this, char *_buf, unsigned int _size);
    int                 (*sendAudioEx)(void *_this,
									   unsigned int  _encoding_type,
							           unsigned char _channel,
							           unsigned char _bit_per_sample,
							           unsigned int  _sampling_rate,
									   char *_buf,
									   unsigned int _size);

    // See below for details...
    int                 (*sendControl)(void *_this, unsigned int _type, char *_buf, unsigned int _size);
    int                 (*doStateMachine)(void *_this);
    int                 (*select)(void *_this, unsigned int _sec, unsigned int _usec);
    int                 (*setConnectTimeout)(void *_this, unsigned int _sec);
} CNetSession, *pCNetSession;


/**************************************************************************************
                        _type values for sendControl
***************************************************************************************/
#define FCC_DOUTSTATUS      MAKEFOURCC('D', 'O', 'S', 'T')

#define FCC_STOPEVENTVIDEO  MAKEFOURCC('E', 'V', 'E', 'N')

    /* these are the _type */
#define FCC_PCONTROL        MAKEFOURCC('P', 'C', 'T', 'L')
/* See below for more information */
#define FCC_CH_BIT          MAKEFOURCC('V', 'B', 'I', 'T')
// _buf : 4-byte-double-word-type bitrate value.
#define FCC_CH_RESOL        MAKEFOURCC('V', 'R', 'E', 'S')
// _buf : 4-byte-double-word-type resolution value.
// Requested resolution type
    #define RESOL_MODE_D1           0x01    // PAL : 720x576,   NTSC : 720x480
    #define RESOL_MODE_4CIF         0x02    // PAL : 704x576,   NTSC : 704x480
    #define RESOL_MODE_VGA          0x03    // PAL : 640x480,   NTSC : 640x480
    #define RESOL_MODE_CIF          0x04    // PAL : 352x288,   NTSC : 352x240
    #define RESOL_MODE_QVGA         0x05    // PAL : 320x240,   NTSC : 320x240
    #define RESOL_MODE_QCIF         0x06    // PAL : 176x144,   NTSC : 176x120
    #define RESOL_MODE_QQVGA        0x07    // PAL : 160x112,   NTSC : 160x112
    #define RESOL_MODE_720P			0x08    // 1280x720
    #define RESOL_MODE_HD1			0x09    // PAL : 720x288,   NTSC : 720x240
    #define RESOL_MODE_H4CIF        0x0A    // PAL : 704x288,   NTSC : 704x240
    #define RESOL_MODE_HVGA			0x0B    // PAL : 640x240,   NTSC : 640x240
    #define RESOL_MODE_SXGA         0x0C    // 1280x1024
    #define RESOL_MODE_UXGA			0x0D    // 1600x1200
    #define RESOL_MODE_1080P		0x0E    // 1920x1080
#define FCC_CH_FRAMEMODE    MAKEFOURCC('V', 'F', 'R', 'M')  
// _buf : 4-byte-double-word-type capture-frame-mode value.
//  #define VIFM_IONLY      0x01
//  #define VIFM_IPFRAME    0x02
//  #define VIFM_IPB        0x03
#define FCC_SERVERFPS       MAKEFOURCC('S', 'F', 'P', 'S')
// _buf : 4-byte-double-word-type server fps.
#define FCC_DIGITALOUT      MAKEFOURCC('D', 'O', 'U', 'T')
// _buf : 4-byte-double-word-type digital-out.
#define FCC_GET_FRAMEMODE   MAKEFOURCC('V', 'G', 'F', 'M')
// _buf : 4-byte-double-word-type get-frame-mode value.
//  #define VIFM_IONLY      0x01
//  #define VIFM_IPFRAME    0x02
//  #define VIFM_IPB        0x03
#define FCC_CH_VIDEO_GROUP  MAKEFOURCC('V', 'G', 'R', 'S')
// _buf : 4-byte-double-word-type group-size value.
// 1 ~ 30
#define FCC_CH_ENCODINGTYPE MAKEFOURCC('V', 'E', 'N', 'C')
// _buf : 4-byte-double-FOURCC value.
#define FCC_AUDIOSET        MAKEFOURCC('A', 'S', 'E', 'T')
// _buf : 4-byte-double-word-type group-size value.
    typedef struct _tagAUDIOFORMATSET
    {
        unsigned int encodingType;  // FOURCC encoding type.
        unsigned int channel;       // audio channels
        unsigned int bitPerSample;  // bit per sample.
        unsigned int samplingRate; // samples per second.
    }AUDIOFORMATSET;
#define FCC_WINPOSSET       MAKEFOURCC('W', 'S', 'E', 'T')
    typedef struct _tagWINPOSSET
    {
        unsigned char   x1; // x coordinator of channel #1
        unsigned char   y1; // y coordinator of channel #1
        unsigned char   w1; // width size of channel #1
        unsigned char   h1; // height size of channel #1
        unsigned char   x2; // x coordinator of channel #2
        unsigned char   y2; // y coordinator of channel #2
        unsigned char   w2; // width size of channel #2
        unsigned char   h2; // height size of channel #2
        unsigned char   x3; // x coordinator of channel #3
        unsigned char   y3; // y coordinator of channel #3
        unsigned char   w3; // width size of channel #3
        unsigned char   h3; // height size of channel #3
        unsigned char   x4; // x coordinator of channel #4
        unsigned char   y4; // y coordinator of channel #4
        unsigned char   w4; // width size of channel #4
        unsigned char   h4; // height size of channel #4
        unsigned char   C1; // do not use. reserved.
        unsigned char   C2; // do not use. reserved.
        unsigned char   C3; // do not use. reserved.
        unsigned char   C4; // do not use. reserved.
        unsigned char   pip;    // picture-in-picture setting
    }WINPOSSET;

#define FCC_SIMPLEWINPOSSET     MAKEFOURCC('S', 'W', 'S', 'T')
    typedef struct _tagSIMPLEWINPOSSET
    {
    #define SIMWPOS_CH1                 1       // ch #1 only
    #define SIMWPOS_CH2                 2       // ch #2 only
    #define SIMWPOS_CH3                 3       // ch #3 only
    #define SIMWPOS_CH4                 4       // ch #4 only
    #define SIMWPOS_QUAD                5       // All channels at the same time.
    #define SIMWPOS_PIP                 6       // if this is set, the PIPOption must be set too.
        unsigned char nType;        // refer to the upper values...
        struct
        {
            unsigned char nHostChannel;     // The host channel number that is shown in a full screen
            unsigned char nEmbChannel;      // The embeded channel number to the host channel.
    #define SIMWPOS_EMBPOS_TOPLEFT      1
    #define SIMWPOS_EMBPOS_TOPRIGHT     2
    #define SIMWPOS_EMBPOS_BOTTOMRIGHT  3
    #define SIMWPOS_EMBPOS_BOTTOMLEFT   4
            unsigned char nEmbLocation;     // The position of the embeded channel.
    #define SIMWPOS_EMBSCALE_HALF       1
    #define SIMWPOS_EMBSCALE_THIRD      2
    #define SIMWPOS_EMBSCALE_QUARTER    3
            unsigned char nEmbScale;            // How big is the embeded channel?
        }PIPOption;
    }SIMPLEWINPOSSET;


#define FCC_AVONOFF         MAKEFOURCC('S', 'T', 'Y', 'P')
    #define STYP_VIDEO      0x00000001
    #define STYP_AUDIO      0x00000002
    #define STYP_SENDAUDIO  0x00000004
// _buf : 4-byte-double-word-type value.
//        0x01 : Video, 0x02 : Sampling Audio, 0x04 : Playback Audio.


/*
    When Control id is
    #define FCC_PCONTROL        'LTCP'

    The control ids are here...

    The Daemon Control IDs are used to control the Pan-Tilt, Zoom, Focus and so forth of the specified camera source.

    * grammar *
    pNetSession->sendControl(
        pNetSesson,             // pNetSession is a return value of the NETSESSION_Init() function. and this pointer must not point NULL.
        FCC_PCONTROL,           // this indicates the content to send is a daemon-control.
        _buf,                   // pointer to a buffer that keeps control data.
        size                    // the size of the _buf. refer to the control id for the size.
    );

*/

    typedef struct _tagCONTROLDATA
    {
        unsigned short channel;
        unsigned short controlID;
        union 
        {
            unsigned char brighness;
            unsigned char contrast;
            unsigned char saturation;
            unsigned char sharpness;
            unsigned char whitebalance;
            unsigned char blc;
            unsigned char zoom;
            unsigned char focus;
            unsigned char exposure;
            unsigned char iris;
            unsigned char pan;
            unsigned char tilt;
            unsigned char light;
            unsigned char preset_set;
            unsigned char preset_move;
            unsigned char preset_clear;
            char  focus_cont;
            struct
            {
                char pan;
                char tilt;
                char zoom;
            }ptz_cont;
            unsigned char transparent_data[1024];
        }payload;
    }CONTROLDATA;


    #define PCTL_BRIGHTNESS             0x1001      // 
    #define PCTL_CONTRAST               0x1002      // 
    #define PCTL_SATURATION             0x1003      // 
    #define PCTL_SHARPNESS              0x1004      // 
    #define PCTL_WHITEBALANCE           0x1005      // 
    #define PCTL_BLC                    0x1006      // backlight compensation
    #define PCTL_ZOOM                   0x1007      // 
    #define PCTL_FOCUS                  0x1008      // 
    #define PCTL_EXPOSURE               0x1009      // 
    #define PCTL_IRIS                   0x100A      // 
    #define PCTL_PAN                    0x100B      // 
    #define PCTL_TILT                   0x100C      // 
    #define PCTL_LIGHT                  0x1011      // light on/off
    #define PCTL_PRESET_SET             0x1020      // 
    #define PCTL_PRESET_MOVE            0x1021      // 
    #define PCTL_PRESET_CLEAR           0x1022      // 
    #define PCTL_FOCUS_CONT             0x2001      // 
    #define PCTL_PTZ_CONT               0x2002      // 
    #define PCTL_RESETDOME              0xFFFF
    #define PCTL_TRANSPARENT            0x7000      // Set the control-daemon to transparent mode...
    #define PCTL_TRANSPARENT_RS232      0x7001      // Set the control-daemon to transparent mode...




#ifndef _NVS_VERSION_STRUCTURE_
#define _NVS_VERSION_STRUCTURE_
// Call to get the version of NVSNetLib,
typedef struct NVSVersion {
    int major;
    int minor;
    int release;
    int build;
} NVSVersion, *pNVSVersion;
#endif



/*----------------------------------------------------------------------------*/
NVSNETLIB_API void * NETSESSION_Init(
    char            *_address, 
    unsigned short  _port, 
    char            *_url,
    char            *_id,
    char            *_passwd,
    unsigned char   _connectType
    );

// for MR904
NVSNETLIB_API void * NETSESSION_InitEx(
    char            *_address, 
    unsigned short  _port, 
    unsigned char   _which_ch,
    char            *_url,
    char            *_id,
    char            *_passwd,
    unsigned char   _connectType
    );

NVSNETLIB_API NVSVersion NETSESSION_GetVersion();
    

#endif

