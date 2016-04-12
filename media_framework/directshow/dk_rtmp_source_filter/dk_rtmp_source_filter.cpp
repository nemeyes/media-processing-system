#include <winsock2.h>
#include <windows.h>

#include <tchar.h>
#include <dshow.h>
#include <initguid.h> 
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <atlbase.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>

#include <atlbase.h>
#include <atlstr.h>
#include <comutil.h>

#include <dk_string_helper.h>
#include <dk_media_buffering.h>
#include "dk_rtmp_source_filter.h"
#include "dk_rtmp_video_source_stream.h"
#include "dk_rtmp_audio_source_stream.h"

dk_rtmp_source_filter::dk_rtmp_source_filter(LPUNKNOWN unk, HRESULT *hr)
	: CSource(g_sz_filter_name, unk, CLSID_DK_RTMP_SOURCE_FILTER)
{
	CAutoLock cAutoLock(&m_cStateLock);
	new dk_rtmp_video_source_stream(hr, this, L"Video");
	new dk_rtmp_audio_source_stream(hr, this, L"Audio");
}

dk_rtmp_source_filter::~dk_rtmp_source_filter(void)
{

}

CUnknown* WINAPI dk_rtmp_source_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	CUnknown *punk = new dk_rtmp_source_filter(unk, hr);
	if (punk == NULL)
	{
		*hr = E_OUTOFMEMORY;
	}
	return punk;
}

STDMETHODIMP dk_rtmp_source_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IFileSourceFilter)
	{
		return GetInterface(static_cast<IFileSourceFilter*>(this), ppv);
	}
	else if (riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface(static_cast<IAMFilterMiscFlags*>(this), ppv);
	}
	else if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else if (riid == IID_IRTMPClient)
	{
		return GetInterface(static_cast<IRTMPClient*>(this), ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}

STDMETHODIMP dk_rtmp_source_filter::Run(REFERENCE_TIME start)
{
	CAutoLock cObjectLock(m_pLock);

	// remember the stream time offset
	m_tStart = start;

	if (m_State == State_Stopped)
	{
		HRESULT hr = Pause();
		if (FAILED(hr))
			return hr;
	}
	// notify all pins of the change to active state
	if (m_State != State_Running)
	{
		int cPins = GetPinCount();
		for (int c = 0; c<cPins; c++)
		{
			CBasePin *pPin = GetPin(c);
			if (NULL == pPin) break;

			// Disconnected pins are not activated - this saves pins
			// worrying about this state themselves

			if (pPin->IsConnected())
			{
				HRESULT hr = pPin->Run(start);
				if (FAILED(hr))  return hr;
			}
		}
	}
	m_State = State_Running;
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::Stop(void)
{
	CAutoLock cObjectLock(m_pLock);
	HRESULT hr = NOERROR;

	// notify all pins of the state change
	if (m_State != State_Stopped)
	{
		_subscriber.stop();

		int cPins = GetPinCount();
		for (int c = 0; c<cPins; c++)
		{
			CBasePin *pPin = GetPin(c);
			if (NULL == pPin)
				break;

			// Disconnected pins are not activated - this saves pins worrying
			// about this state themselves. We ignore the return code to make
			// sure everyone is inactivated regardless. The base input pin
			// class can return an error if it has no allocator but Stop can
			// be used to resync the graph state after something has gone bad

			if (pPin->IsConnected())
			{
				HRESULT hrTmp = pPin->Inactive();
				if (FAILED(hrTmp) && SUCCEEDED(hr))
				{
					hr = hrTmp;
				}
			}
		}
	}

	m_State = State_Stopped;
	return hr;
}

STDMETHODIMP dk_rtmp_source_filter::Pause(void)
{
	CAutoLock cObjectLock(m_pLock);

	// notify all pins of the change to active state
	if (m_State == State_Stopped)
	{
		//dk_rtmp_subscriber::ERR_CODE code = _subscriber.play();
		//if (code != dk_rtmp_subscriber::ERR_CODE_SUCCESS)
		//	return E_FAIL;

		int cPins = GetPinCount();
		for (int c = 0; c<cPins; c++)
		{
			CBasePin *pPin = GetPin(c);
			if (NULL == pPin) break;

			// Disconnected pins are not activated - this saves pins
			// worrying about this state themselves

			if (pPin->IsConnected())
			{
				HRESULT hr = pPin->Active();
				if (FAILED(hr)) return hr;
			}
		}
	}
	m_State = State_Paused;
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetCurFile(LPOLESTR *file_name, AM_MEDIA_TYPE *type)
{
	wchar_t * url = 0;
	_subscriber.get_url(&url);
	if (url)
	{
		DWORD n = sizeof(WCHAR)*(1 + lstrlenW(url));
		*file_name = (LPOLESTR)CoTaskMemAlloc(n);
		CopyMemory(*file_name, url, n);
		free(url);
		return NOERROR;
	}
	else
	{
		return E_FAIL;
	}
}

STDMETHODIMP dk_rtmp_source_filter::Load(LPCOLESTR file_name, const AM_MEDIA_TYPE *type)
{
	_subscriber.play();
	//for (int32_t i = 0; i < 600; i++)
	while (true)
	{
		if (_subscriber.state() == dk_rtmp_client::STATE_STOPPED)
			return E_UNEXPECTED;

		uint16_t recv_option = 0;
		_subscriber.get_recv_option(recv_option);
		if ((recv_option & DK_RECV_VIDEO) && (recv_option & DK_RECV_AUDIO))
		{
			buffering::vsubmedia_type vmt = buffering::unknown_video_type;
			dk_media_buffering::instance().get_video_submedia_type(vmt);

			buffering::asubmedia_type amt = buffering::unknown_audio_type;
			dk_media_buffering::instance().get_audio_submedia_type(amt);

			int32_t video_width = 0, video_height = 0;
			dk_media_buffering::instance().get_video_width(video_width);
			dk_media_buffering::instance().get_video_height(video_height);

			int32_t audio_samplerate = 0, audio_bitdepth = 0, audio_channels = 0;
			dk_media_buffering::instance().get_audio_samplerate(audio_samplerate);
			dk_media_buffering::instance().get_audio_bitdepth(audio_bitdepth);
			dk_media_buffering::instance().get_audio_channels(audio_channels);

			if ((vmt != buffering::unknown_video_type) &&
				(amt != buffering::unknown_audio_type) &&
				(video_width > 0) && (video_height > 0) &&
				(audio_samplerate>0) &&
				(audio_bitdepth>0) &&
				(audio_channels>0))
			{
				break;
			}
		}
		else if (recv_option & DK_RECV_VIDEO)
		{
			buffering::vsubmedia_type vmt = buffering::unknown_video_type;
			dk_media_buffering::instance().get_video_submedia_type(vmt);

			int32_t video_width = 0, video_height = 0;
			dk_media_buffering::instance().get_video_width(video_width);
			dk_media_buffering::instance().get_video_height(video_height);

			if ((vmt != buffering::unknown_video_type) && (video_width > 0) && (video_height > 0))
			{
				break;
			}
		}
		else if (recv_option & DK_RECV_AUDIO)
		{
			buffering::asubmedia_type amt = buffering::unknown_audio_type;
			dk_media_buffering::instance().get_audio_submedia_type(amt);

			int32_t audio_samplerate = 0, audio_bitdepth = 0, audio_channels = 0;
			dk_media_buffering::instance().get_audio_samplerate(audio_samplerate);
			dk_media_buffering::instance().get_audio_bitdepth(audio_bitdepth);
			dk_media_buffering::instance().get_audio_channels(audio_channels);

			if ((amt != buffering::unknown_audio_type) && (audio_samplerate>0) && (audio_bitdepth>0) && (audio_channels>0))
			{
				break;
			}
		}

		::Sleep(1);
	}
	return S_OK;
}

ULONG STDMETHODCALLTYPE dk_rtmp_source_filter::GetMiscFlags(void)
{
	return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

STDMETHODIMP dk_rtmp_source_filter::GetPages(CAUUID * pages)
{
	if (pages == NULL)
		return E_POINTER;
	pages->cElems = 1;
	pages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pages->pElems == NULL)
		return E_OUTOFMEMORY;
	pages->pElems[0] = CLSID_DK_RTMP_SOURCE_FILTER_PROPERTIES;
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetUrl(BSTR url)
{
	_subscriber.set_url(url);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetUsername(BSTR username)
{
	_subscriber.set_username(username);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetPassword(BSTR password)
{
	_subscriber.set_password(password);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetRecvOption(USHORT option)
{
	uint16_t recv_option = dk_rtmp_client::RECV_OPTION_NONE;
	if (option & DK_RECV_VIDEO)
		recv_option |= dk_rtmp_client::RECV_OPTION_VIDEO;
	if (option & DK_RECV_AUDIO)
		recv_option |= dk_rtmp_client::RECV_OPTION_AUDIO;

	_subscriber.set_recv_option(option);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetRecvTimeout(ULONGLONG timeout)
{
	_subscriber.set_recv_timeout(timeout);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetConnectionTimeout(ULONGLONG timeout)
{
	_subscriber.set_connection_timeout(timeout);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::SetRepeat(BOOL repeat)
{
	_subscriber.set_repeat(repeat==TRUE?true:false);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetUrl(BSTR * url)
{
	wchar_t * tmp_url = 0;
	_subscriber.get_url(&tmp_url);
	if (tmp_url)
	{
		//*url = _bstr_t(tmp_url);
		free(tmp_url);
	}
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetUsername(BSTR * username)
{
	wchar_t * tmp_username = 0;
	_subscriber.get_username(&tmp_username);
	if (tmp_username)
	{
		//*username = _bstr_t(tmp_username);
		free(tmp_username);
	}
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetPassword(BSTR * password)
{
	wchar_t * tmp_password = 0;
	_subscriber.get_password(&tmp_password);
	if (tmp_password)
	{
		//*password = _bstr_t(tmp_password);
		free(tmp_password);
	}
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetRecvOption(USHORT & option)
{
	_subscriber.get_recv_option(option);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetRecvTimeout(ULONGLONG & timeout)
{
	_subscriber.get_recv_timeout(timeout);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetConnectionTimeout(ULONGLONG & timeout)
{
	_subscriber.get_connection_timeout(timeout);
	return S_OK;
}

STDMETHODIMP dk_rtmp_source_filter::GetRepeat(BOOL & repeat)
{
	bool tmp_repeat = false;
	_subscriber.get_repeat(tmp_repeat);
	repeat = tmp_repeat == TRUE ? true : false;
	return S_OK;
}