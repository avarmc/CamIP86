/*
 * copyright (c) 2015 Elsys Corp.
 *
 * This file is part of CamIP.
 *
 * CamIP is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * CamIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CamIP; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */ 

#include "stdafx.h"


#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include <streams.h>
#include <stdio.h>
#include <olectl.h>
#include <dvdmedia.h>
#include "filters.h"
#include "reg.h"

#define title L""

extern CFactoryTemplate g_Templates[10];


//////////////////////////////////////////////////////////////////////////
//  CCamIP is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI CCamIP::CreateInstanceX(LPUNKNOWN lpunk, HRESULT *phr,int id)
{
    ASSERT(phr);
    CUnknown *punk = new CCamIP(lpunk, phr, id);
    return punk;
}

CCamIP::CCamIP(LPUNKNOWN lpunk, HRESULT *phr, int id) : 
	CSource((WCHAR*)(g_Templates[id].m_Name), lpunk, *g_Templates[id].m_ClsID)
{
	ASSERT(phr);

	m_cfg = ReadConfig(id);

	// Create the one and only output pin
    m_paStreams = (CSourceStream **) new CCamIPStream*[1];
    m_paStreams[0] = new CCamIPStream(phr, this, g_Templates[id].m_Name);
}

HRESULT CCamIP::QueryInterface(REFIID riid, void **ppv)
{
	//Forward request for IAMStreamConfig & IKsPropertySet to the pin
    if(riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet))
        return m_paStreams[0]->QueryInterface(riid, ppv);
    else
        return CSource::QueryInterface(riid, ppv);
}

bool	CCamIP::GetName( LPWSTR name, int id, int strMax )
{
	CAM_CONFIG cfg = ReadConfig(id);
	wcscpy_s(name,strMax,cfg.name );
	return _tcslen( name ) > 0;
}

CCamIP::CAM_CONFIG CCamIP::ReadConfig(int id)
{
	CAM_CONFIG cfg;
	WCHAR reg[256];

	_stprintf_s(reg,256,L"SOFTWARE\\ELSYS\\CamIP\\%0.2u",id);

	cfg.id = id;
	cfg.clsid = g_Templates[id].m_ClsID;
	cfg.fps = GetProfileFloatCmd(reg,L"fps",25);
	cfg.w = GetProfileIntCmd(reg,L"w",640);
	cfg.h = GetProfileIntCmd(reg,L"h",480);
	cfg.enable = GetProfileIntCmd(reg,L"enable",0);

	cfg.rs.left = GetProfileIntCmd(reg,L"crop_l",0);
	cfg.rs.right = GetProfileIntCmd(reg,L"crop_r",0);
	cfg.rs.top = GetProfileIntCmd(reg,L"crop_t",0);
	cfg.rs.bottom = GetProfileIntCmd(reg,L"crop_b",0);

	cfg.tShift = GetProfileIntCmd(reg,L"tShift",0);

	GetProfileStringCmd(reg,L"url",cfg.url,sizeof(cfg.url));
	if( GetProfileStringCmd(reg,L"name",cfg.name,sizeof(cfg.name)) <= 0)
		_stprintf_s(cfg.name,256,L"CamIP %0.2u",id+1);

	return cfg;
}

bool	CCamIP::IsOK(int id)
{
	return  IsOK( ReadConfig(id) );
}

bool	CCamIP::IsOK(const CAM_CONFIG& cfg)
{
	if( !cfg.enable )
		return false;
	if( _tcslen( cfg.url ) < 6 )
		return false;
	if( _tcslen( cfg.name ) <= 0  )
		return false;
	if( cfg.w <= 0 || cfg.h <= 0)
		return false;
	if( cfg.fps <= 0 )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// CCamIPStream is the one and only output pin of CCamIP which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
CCamIPStream::CCamIPStream(HRESULT *phr, CCamIP *pParent, LPCWSTR pPinName) :
    CSourceStream(NAME("Virtual Cam"),phr, pParent, pPinName),
		m_pParent(pParent),
		m_bInit(false),
		m_pIPCam(0),
		m_img(0),
		m_fps( pParent->m_cfg.fps )
{
	m_imgSize.cx = m_imgSize.cy = 0;

	Resize( pParent->m_cfg.w,pParent->m_cfg.h );

	GetMediaType(0, &m_mt);

}

CCamIPStream::~CCamIPStream()
{
	if(m_pIPCam)
		delete m_pIPCam;
	if(m_img)
		_aligned_free(m_img);
} 

HRESULT CCamIPStream::QueryInterface(REFIID riid, void **ppv)
{   
	// Standard OLE stuff
    if(riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*)this;
    else if(riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*)this;
    else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef();
    return S_OK;
}


void CCamIPStream::Resize(int w,int h)
{
	if( m_img && w == m_imgSize.cx && h == m_imgSize.cy )
		return;

	m_imgSize.cx = w;
	m_imgSize.cy = h;

	int w34 = (w*3+3)&(~3);
	size_t s = w34*h;

	m_img = (BYTE*)_aligned_malloc(((s+31)&(~31)),32);
}


//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//////////////////////////////////////////////////////////////////////////

HRESULT CCamIPStream::FillBuffer(IMediaSample *pms)
{
	REFERENCE_TIME rtNow;
    
    REFERENCE_TIME avgFrameTime = ((VIDEOINFOHEADER*)m_mt.pbFormat)->AvgTimePerFrame;

    rtNow = m_rtLastTime;
    m_rtLastTime += avgFrameTime;
    pms->SetTime(&rtNow, &m_rtLastTime);

	if(rtNow >= 0)
		pms->SetSyncPoint(TRUE);
	else
		pms->SetDiscontinuity(TRUE);

	if( m_img && pms)
	{
		CAutoLock lock(&m_lock);

		// crop and resize
		BYTE *pData;

		pms->GetPointer(&pData);

		if(!pData)
			return E_FAIL;



		int ws = m_imgSize.cx;
		int hs = m_imgSize.cy;
		int ws34 = (ws*3+3)&(~3);
		
		memcpy( pData, m_img, ws34*hs );

	}
    return NOERROR;
} // FillBuffer


//
// Notify
// Ignore quality management messages sent from the downstream filter
STDMETHODIMP CCamIPStream::Notify(IBaseFilter * pSender, Quality q)
{
    return E_NOTIMPL;
} // Notify

//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
//////////////////////////////////////////////////////////////////////////
HRESULT CCamIPStream::SetMediaType(const CMediaType *pmt)
{
	DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());
    HRESULT hr = CSourceStream::SetMediaType(pmt);
    return hr;
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT CCamIPStream::GetMediaType(int iPosition, CMediaType *pmt)
{
	if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 1) return VFW_S_NO_MORE_ITEMS;

	if(m_bInit && iPosition == 0 && m_mt.GetSampleSize() > 0) 
    {
        *pmt = m_mt;
        return S_OK;
    }

    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biBitCount    = 24;
    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth      = m_imgSize.cx;
    pvi->bmiHeader.biHeight     = m_imgSize.cy;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

	pvi->AvgTimePerFrame = (REFERENCE_TIME)(0.5+(1.0/m_fps)/100.0e-9);

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
    
	m_bInit = true;
    return NOERROR;

} // GetMediaType

// This method is called to see if a given output format is supported
HRESULT CCamIPStream::CheckMediaType(const CMediaType *pMediaType)
{
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pMediaType->Format());
	if(pvi->bmiHeader.biWidth != m_imgSize.cx)
        return E_FAIL;
	if(pvi->bmiHeader.biHeight != m_imgSize.cy)
        return E_FAIL;
    if(*pMediaType != m_mt) 
        return E_INVALIDARG;
    return S_OK;
} // CheckMediaType

// This method is called after the pins are connected to allocate buffers to stream data
HRESULT CCamIPStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    HRESULT hr = NOERROR;

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);

    if(FAILED(hr)) return hr;
    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;

    return NOERROR;
} // DecideBufferSize




HRESULT CCamIPStream::IPCamConnect()
{
	if( m_pIPCam && m_pIPCam->IsConnected() && m_pIPCam->IsStarted() )
	{
		return S_FALSE;
	}

	if(!m_pIPCam)
	{
		m_pIPCam = new CRTSPCamera(this);
	}

	if( !m_pIPCam || !m_pIPCam->Start( m_pParent->m_cfg.url ) )
	{
		return E_FAIL;
	}

	return S_OK;
}

bool CCamIPStream::OnImage(void *p,int w,int h)
{
	CAutoLock cAutoLock(&m_lock);

	int wd = m_imgSize.cx;
	int hd = m_imgSize.cy;
	int ws = w;
	int hs = h;

	int w34d = (wd*3+3)&(~3);
	int w34s = (ws*3+3)&(~3);

	RECT rs = { 0,0, ws,hs };

	CCamIP::CAM_CONFIG & cfg = m_pParent->m_cfg;

	if( cfg.rs.top || cfg.rs.bottom )
	{
		rs.top = min(cfg.rs.top,cfg.rs.bottom);
		rs.bottom = max(cfg.rs.top,cfg.rs.bottom);
		rs.top = max( rs.top, 0 );
		rs.bottom = max( rs.bottom, 0 );
		rs.top = min( rs.top, hs );
		rs.bottom = min( rs.bottom, hs );
	}
	if( cfg.rs.left || cfg.rs.right )
	{
		rs.left = min(cfg.rs.left,cfg.rs.right);
		rs.right = max(cfg.rs.left,cfg.rs.right);
		rs.left = max( rs.left, 0 );
		rs.right = max( rs.right, 0 );
		rs.left = min( rs.left, ws );
		rs.right = min( rs.right, ws );
	}

	int rsw = rs.right-rs.left;
	int rsh = rs.bottom-rs.top;

	if(!rsw || !rsh)
		memset( m_img, 0, w34d*hd );
	else
	{ 
		for( int yd = 0; yd < hd; ++yd )
		{
			RGBTRIPLE *pyd = (RGBTRIPLE *)(m_img+yd*w34d);

			int ys = hs-1-(yd*rsh/hd + rs.top);
			if( ys < 0 || ys >= hs )
				continue;
			RGBTRIPLE *pys = (RGBTRIPLE *)(((BYTE*)p)+ys*w34s);

			for( int xd = 0; xd < wd; ++xd )
			{
				int xs = xd*rsw/wd + rs.left;
				if( xs < 0 || xs >= ws )
					continue;

				pyd[xd] = pys[xs];
			}
		}
	}
	return true;
}

HRESULT CCamIPStream::OnThreadDestroy(void)
{
	if(m_pIPCam)
	{
		m_pIPCam->Stop();
		m_pIPCam->Disconnect();
	}
	return S_OK;
}

// Called when graph is run
HRESULT CCamIPStream::OnThreadCreate()
{
	CAutoLock cAutoLock(&m_lock);

	m_rtLastTime = 0;

	IPCamConnect();
	if(m_pParent)
		m_rtLastTime = m_pParent->m_cfg.tShift*(LONGLONG)10000;

    return NOERROR;
} // OnThreadCreate


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CCamIPStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
 	DECLARE_PTR(VIDEOINFOHEADER, pvi, m_mt.pbFormat);
 	DECLARE_PTR(VIDEOINFOHEADER, npvi, pmt->pbFormat);

	if(npvi->bmiHeader.biWidth != m_imgSize.cx)
        return VFW_E_INVALIDMEDIATYPE;
	if(npvi->bmiHeader.biHeight != m_imgSize.cy)
        return VFW_E_INVALIDMEDIATYPE;	

    m_mt = *pmt;
	Resize(pvi->bmiHeader.biWidth,pvi->bmiHeader.biHeight);
    IPin* pin; 
    ConnectedTo(&pin);
    if(pin)
    {
        IFilterGraph *pGraph = m_pParent->GetGraph();
        pGraph->Reconnect(this);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCamIPStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
 	*ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCamIPStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	*piCount = 1;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCamIPStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
	*pmt = CreateMediaType(&m_mt);
    DECLARE_PTR(VIDEOINFOHEADER, pvi, (*pmt)->pbFormat);

    pvi->bmiHeader.biCompression = BI_RGB;
    pvi->bmiHeader.biBitCount    = 24;
    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth      = m_imgSize.cx;
    pvi->bmiHeader.biHeight     = m_imgSize.cy;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    (*pmt)->majortype = MEDIATYPE_Video;
    (*pmt)->subtype = MEDIASUBTYPE_RGB24;
    (*pmt)->formattype = FORMAT_VideoInfo;
    (*pmt)->bTemporalCompression = FALSE;
    (*pmt)->bFixedSizeSamples= FALSE;
    (*pmt)->lSampleSize = pvi->bmiHeader.biSizeImage;
    (*pmt)->cbFormat = sizeof(VIDEOINFOHEADER);
    
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
    
    pvscc->guid = FORMAT_VideoInfo;
    pvscc->VideoStandard = AnalogVideo_None;
    pvscc->InputSize.cx = m_imgSize.cx;
    pvscc->InputSize.cy = m_imgSize.cy;
    pvscc->MinCroppingSize.cx = 80;
    pvscc->MinCroppingSize.cy = 60;
    pvscc->MaxCroppingSize.cx = 1920;
    pvscc->MaxCroppingSize.cy = 1080;
    pvscc->CropGranularityX = 4;
    pvscc->CropGranularityY = 4;
    pvscc->CropAlignX = 0;
    pvscc->CropAlignY = 0;

    pvscc->MinOutputSize.cx = 80;
    pvscc->MinOutputSize.cy = 60;
    pvscc->MaxOutputSize.cx = 1920;
    pvscc->MaxOutputSize.cy = 1080;
    pvscc->OutputGranularityX = 0;
    pvscc->OutputGranularityY = 0;
    pvscc->StretchTapsX = 0;
    pvscc->StretchTapsY = 0;
    pvscc->ShrinkTapsX = 0;
    pvscc->ShrinkTapsY = 0;
    pvscc->MinFrameInterval = 200000;   //50 fps
    pvscc->MaxFrameInterval = 50000000; // 0.2 fps
    pvscc->MinBitsPerSecond = (80 * 60 * 3 * 8) / 5;
    pvscc->MaxBitsPerSecond = 1920L * 1080L * 3L * 8L * 25;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////


HRESULT CCamIPStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{// Set: Cannot set any properties.
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CCamIPStream::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
)
{
    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.
        
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CCamIPStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}
