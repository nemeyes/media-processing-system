#pragma once
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>

class dk_haali_media_splitter
{
public:
	dk_haali_media_splitter(void);
	~dk_haali_media_splitter(void);

	HRESULT add_to_graph(CComPtr<IGraphBuilder> graph, wchar_t * file);
	CComPtr<IPin> get_outpin(void);

private:
	CComPtr<IBaseFilter> _source;
};