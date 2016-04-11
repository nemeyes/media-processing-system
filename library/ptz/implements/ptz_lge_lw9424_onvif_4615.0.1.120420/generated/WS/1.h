#ifndef _AV_SOURCE_H_
#define _AV_SOURCE_H_

#if defined(WIN32)
# include <windows.h>
# include <streams.h>
# include <atlbase.h>
# include <atlconv.h>
# pragma include_alias( "dxtrans.h", "qedit.h" )
# define __IDxtCompositor_INTERFACE_DEFINED__
# define __IDxtAlphaSetter_INTERFACE_DEFINED__
# define __IDxtJpeg_INTERFACE_DEFINED__
# define __IDxtKey_INTERFACE_DEFINED__
# include <qedit.h>
# define STRSAFE_NO_DEPRECATE
# include <strsafe.h>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}


typedef struct _callbackinfo 
{
	double dblSampleTime;
	long lBufferSize;
	BYTE *pBuffer;

	// Video Info
    BITMAPINFOHEADER bih;
	DWORD biSize;
	DWORD           dwBitRate;         // Approximate bit data rate
	DWORD           dwBitErrorRate;    // Bit error rate for this stream
	REFERENCE_TIME  AvgTimePerFrame;   // Average time per frame (100ns units)

	// Audio Info
	WAVEFORMATEX wfh;
} CALLBACKINFO;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(WIN32)
class AVSource : public ISampleGrabberCB
#else
class AVSource
#endif
{
public:
	explicit AVSource( void );
	~AVSource( void );

#if defined(WIN32)
public:
	void Initialize( void* Streamer, int SrcMediaType, int SrcMediaCodec, int ScrWidth=0, int ScrHeight=0 );
	void Destroy( void );
	void Start( void );
	void Pause( void );
	void Stop( void );
	void SaveBitmap( void );

	void GetDefaultCapDevice( IBaseFilter** cap );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );
    STDMETHODIMP QueryInterface( REFIID riid, void** ppv );
	STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample );
	STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize );
	int CopyBitmap( double SampleTime, BYTE * pBuffer, long BufferSize );
	int CopyJpeg( double SampleTime, BYTE * pJpegBuffer, long pJpegBufferSize );

	int FFMpeg_Encoding_Initialize( int MediaSourceType );
	int FFMpeg_Encoding_Release();

	// FFMPEG Encoding - Rowdata(RGB24) to MJPEG , Return - Encoding size
	int FFMpeg_Rowdata_Rgb24toMJpeg( double SampleTime, BYTE* pBuffer, long BufferSize, BYTE* pOutBuffer, int pOutBufferSize );

	// FFMPEG Encoding - Rowdata(RGB24) to H.264 , Return - Encoding size
	int FFMpeg_Rowdata_Rgb24toH264( double SampleTime, BYTE* pBuffer, long BufferSize, BYTE* pOutBuffer, int pOutBufferSize );

	// FFMPEG Encoding - Rowdata(PCM) to AAC , Return - Encoding size
	int FFMpeg_Rowdata_PcmToAac( double SampleTime, BYTE* pBuffer, long BufferSize, BYTE* pOutBuffer, int pOutBufferSize );
	int FFMpeg_Rowdata_PcmToAac2( double SampleTime, BYTE* pBuffer, long BufferSize, BYTE* pOutBuffer, int pOutBufferSize );

protected:
	HRESULT GetPin( IBaseFilter* filter, PIN_DIRECTION dirrequired, int iNum, IPin **pin );
	IPin*  GetInPin( IBaseFilter* filter, int num );
	IPin*  GetOutPin( IBaseFilter* filter, int num );
	HRESULT SetAudioBuffer(IPin *AudioCapturePin, DWORD bufferSize);

private:
	int _frameCount;
	void* _streamer;
	int _srcMediaType;
	int _srcMediaCodec;
	int _scrWidth;
	int _scrHeight;

	CComPtr<ISampleGrabber> _grabber;
	CComPtr<IBaseFilter>    _source;
	CComPtr<IGraphBuilder>  _graph;
	CComQIPtr<IMediaControl, &IID_IMediaControl> _control;
	CComQIPtr<IMediaEvent, &IID_IMediaEvent> _event;
	CComPtr<IVideoWindow>   _videoWindow;
	CALLBACKINFO _cbInfo;

	long _width;
	long _height;

	unsigned int _iFileWrite;
	BOOL _bFileWrite;

	AVCodec* _avCodec;
	AVCodecContext* _avCtx;

#endif

};

#endif