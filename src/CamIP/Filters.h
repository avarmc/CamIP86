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

#pragma once

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

#include "CamIP_GUID.h"
#include "RTSPCamera.h"

class CCamIPStream;
class CCamIP : public CSource
{
public:
	typedef struct tagCAM_CONFIG
	{
		int		id;
		int		enable;
		const CLSID*	clsid;
		WCHAR	url[2048];
		WCHAR	name[2048];
		float	fps;
		int		w,h;			// dst image size
		RECT	rs;				// crop rect
		int	tShift; // time Shift (ms)
	}CAM_CONFIG;
public:
    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    static CUnknown * WINAPI CreateInstanceX(LPUNKNOWN lpunk, HRESULT *phr,int id);

	static CUnknown * WINAPI CreateInstance0(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,0); }
	static CUnknown * WINAPI CreateInstance1(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,1); }
	static CUnknown * WINAPI CreateInstance2(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,2); }
	static CUnknown * WINAPI CreateInstance3(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,3); }
	static CUnknown * WINAPI CreateInstance4(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,4); }
	static CUnknown * WINAPI CreateInstance5(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,5); }
	static CUnknown * WINAPI CreateInstance6(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,6); }
	static CUnknown * WINAPI CreateInstance7(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,7); }
	static CUnknown * WINAPI CreateInstance8(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,8); }
	static CUnknown * WINAPI CreateInstance9(LPUNKNOWN lpunk, HRESULT *phr) { return CreateInstanceX(lpunk,phr,9); }

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

    IFilterGraph *GetGraph() {return m_pGraph;}
	
public:
	static bool	IsOK(int id);
	static bool	GetName( LPWSTR name, int id, int strMax = 256 );
public:
	CAM_CONFIG	m_cfg;
private:
    CCamIP(LPUNKNOWN lpunk, HRESULT *phr,int id);

	static CAM_CONFIG ReadConfig(int id);
	static bool	IsOK(const CAM_CONFIG& cfg);
};

class CCamIPStream : public CSourceStream, public IAMStreamConfig, public IKsPropertySet, public CIPCameraCallback
{
public:

    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
    STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

    //////////////////////////////////////////////////////////////////////////
    //  IQualityControl
    //////////////////////////////////////////////////////////////////////////
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    //////////////////////////////////////////////////////////////////////////
    //  IAMStreamConfig
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

    //////////////////////////////////////////////////////////////////////////
    //  IKsPropertySet
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
    
    //////////////////////////////////////////////////////////////////////////
    //  CSourceStream
    //////////////////////////////////////////////////////////////////////////
    CCamIPStream(HRESULT *phr,CCamIP *pParent, LPCWSTR pPinName);
    ~CCamIPStream();

    HRESULT FillBuffer(IMediaSample *pms);
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT OnThreadCreate(void);
    HRESULT OnThreadDestroy(void);

	HRESULT IPCamConnect();

	virtual bool OnImage(void *p,int w,int h);

protected:
	void				Resize(int w,int h);
protected:
	CCritSec			m_lock;
	BYTE *				m_img;
	SIZE				m_imgSize;
	float				m_fps;
private:
	CIPCameraBase *		m_pIPCam;		
    CCamIP *			m_pParent;
    REFERENCE_TIME		m_rtLastTime;
    HBITMAP				m_hLogoBmp;
    CCritSec			m_cSharedState;
    IReferenceClock *	m_pClock;
	bool				m_bInit;

};


