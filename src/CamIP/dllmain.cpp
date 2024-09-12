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


//////////////////////////////////////////////////////////////////////////
//  This file contains routines to register / Unregister the 
//  Directshow filter 'Virtual Cam'
//  We do not use the inbuilt BaseClasses routines as we need to register as
//  a capture source
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")

#ifdef _DEBUG
    #pragma comment(lib, "strmbasd")
#else
    #pragma comment(lib, "strmbase")
#endif


#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <dllsetup.h>
#include "filters.h"

#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);

STDAPI AMovieSetupRegisterServer( CLSID   clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType     = L"InprocServer32" );
STDAPI AMovieSetupUnregisterServer( CLSID clsServer );



// {D2A278CE-E16B-43d5-A5E2-AECAFD64DF77}
DEFINE_GUID(CLSID_CamIP, 
0xd2a278ce, 0xe16b, 0x43d5, 0xa5, 0xe2, 0xae, 0xca, 0xfd, 0x64, 0xdf, 0x77);

// {C3A21440-DBD1-478d-B853-76689A9DCC61}
DEFINE_GUID(CLSID_CamIP0, 
0xc3a21440, 0xdbd1, 0x478d, 0xb8, 0x53, 0x76, 0x68, 0x9a, 0x9d, 0xcc, 0x61);

// {0D17080B-9DDD-4c6d-978F-2CDF4DC3C9E2}
DEFINE_GUID(CLSID_CamIP1, 
0xd17080b, 0x9ddd, 0x4c6d, 0x97, 0x8f, 0x2c, 0xdf, 0x4d, 0xc3, 0xc9, 0xe2);

// {E2906275-48A0-467d-B025-CED8F1823B76}
DEFINE_GUID(CLSID_CamIP2, 
0xe2906275, 0x48a0, 0x467d, 0xb0, 0x25, 0xce, 0xd8, 0xf1, 0x82, 0x3b, 0x76);

// {077E3B87-88BE-449f-808B-E6B3308F7B44}
DEFINE_GUID(CLSID_CamIP3, 
0x77e3b87, 0x88be, 0x449f, 0x80, 0x8b, 0xe6, 0xb3, 0x30, 0x8f, 0x7b, 0x44);

// {4DC2A111-F6EE-4f7a-B2F7-79E6ACEECAE7}
DEFINE_GUID(CLSID_CamIP4, 
0x4dc2a111, 0xf6ee, 0x4f7a, 0xb2, 0xf7, 0x79, 0xe6, 0xac, 0xee, 0xca, 0xe7);

// {C3D4C1C7-18CE-4c2f-9FDB-DDB2960FF210}
DEFINE_GUID(CLSID_CamIP5, 
0xc3d4c1c7, 0x18ce, 0x4c2f, 0x9f, 0xdb, 0xdd, 0xb2, 0x96, 0xf, 0xf2, 0x10);

// {EEB8E63D-1D87-49b8-8B35-61FA23F17C3C}
DEFINE_GUID(CLSID_CamIP6, 
0xeeb8e63d, 0x1d87, 0x49b8, 0x8b, 0x35, 0x61, 0xfa, 0x23, 0xf1, 0x7c, 0x3c);

// {5B999BBB-D1F4-43ff-8D75-C11F7B8D280D}
DEFINE_GUID(CLSID_CamIP7, 
0x5b999bbb, 0xd1f4, 0x43ff, 0x8d, 0x75, 0xc1, 0x1f, 0x7b, 0x8d, 0x28, 0xd);

// {3456B19D-B478-4f4a-B869-3A3C9C10EB46}
DEFINE_GUID(CLSID_CamIP8, 
0x3456b19d, 0xb478, 0x4f4a, 0xb8, 0x69, 0x3a, 0x3c, 0x9c, 0x10, 0xeb, 0x46);

// {A79F18FB-979F-43e7-A317-BC92DC1AE36F}
DEFINE_GUID(CLSID_CamIP9, 
0xa79f18fb, 0x979f, 0x43e7, 0xa3, 0x17, 0xbc, 0x92, 0xdc, 0x1a, 0xe3, 0x6f);



const AMOVIESETUP_MEDIATYPE AMSMediaTypesVCam = 
{ 
    &MEDIATYPE_Video, 
    &MEDIASUBTYPE_NULL 
};

const AMOVIESETUP_PIN AMSPinVCam=
{
    L"Output",             // Pin string name
    FALSE,                 // Is it rendered
    TRUE,                  // Is it an output
    FALSE,                 // Can we have none
    FALSE,                 // Can we have many
    &CLSID_NULL,           // Connects to filter
    NULL,                  // Connects to pin
    1,                     // Number of types
    &AMSMediaTypesVCam      // Pin Media types
};

const AMOVIESETUP_FILTER AMSFilterVCam =
{
    &CLSID_CamIP,  // Filter CLSID
    L"CamIP",     // String name
    MERIT_DO_NOT_USE,      // Filter merit
    1,                     // Number pins
    &AMSPinVCam             // Pin details
};


WCHAR g_names[10][256] = 
{
	L"CamIP 1",
	L"CamIP 2",
	L"CamIP 3",
	L"CamIP 4",
	L"CamIP 5",
	L"CamIP 6",
	L"CamIP 7",
	L"CamIP 8",
	L"CamIP 9",
	L"CamIP 10"
};

CFactoryTemplate g_Templates[10] = 
{
    {g_names[0],&CLSID_CamIP0,CCamIP::CreateInstance0,NULL,&AMSFilterVCam},
    {g_names[1],&CLSID_CamIP1,CCamIP::CreateInstance1,NULL,&AMSFilterVCam},
    {g_names[2],&CLSID_CamIP2,CCamIP::CreateInstance2,NULL,&AMSFilterVCam},
    {g_names[3],&CLSID_CamIP3,CCamIP::CreateInstance3,NULL,&AMSFilterVCam},
    {g_names[4],&CLSID_CamIP4,CCamIP::CreateInstance4,NULL,&AMSFilterVCam},
    {g_names[5],&CLSID_CamIP5,CCamIP::CreateInstance5,NULL,&AMSFilterVCam},
    {g_names[6],&CLSID_CamIP6,CCamIP::CreateInstance6,NULL,&AMSFilterVCam},
    {g_names[7],&CLSID_CamIP7,CCamIP::CreateInstance7,NULL,&AMSFilterVCam},
    {g_names[8],&CLSID_CamIP8,CCamIP::CreateInstance8,NULL,&AMSFilterVCam},
    {g_names[9],&CLSID_CamIP9,CCamIP::CreateInstance9,NULL,&AMSFilterVCam}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



STDAPI RegisterFilters( BOOL bRegister )
{
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);

    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                       achFileName, NUMELMS(achFileName));
  
    hr = CoInitialize(0);
	for(int i = 0; i < g_cTemplates; ++i)
	{
		bool ok = CCamIP::IsOK(i);

		if(bRegister)
		{
			hr = ok ? AMovieSetupRegisterServer(*g_Templates[i].m_ClsID, g_Templates[i].m_Name, achFileName, L"Both", L"InprocServer32") : S_OK;
		}

		if( SUCCEEDED(hr) )
		{
			IFilterMapper2 *fm = 0;
			hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, fm );
			if( SUCCEEDED(hr) )
			{
				if(bRegister)
				{
					IMoniker *pMoniker = 0;
					REGFILTER2 rf2;
					rf2.dwVersion = 1;
					rf2.dwMerit = MERIT_DO_NOT_USE;
					rf2.cPins = 1;
					rf2.rgPins = &AMSPinVCam;

					hr = ok ? fm->RegisterFilter(*g_Templates[i].m_ClsID,g_Templates[i].m_Name, &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2) : S_OK;
				}
				else
				{
					hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, *g_Templates[i].m_ClsID);
				}
			}

		  // release interface
		  //
		  if(fm)
			  fm->Release();
		}

		if( SUCCEEDED(hr) && !bRegister )
			hr = AMovieSetupUnregisterServer( *g_Templates[i].m_ClsID );
	}

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

STDAPI DllRegisterServer()
{
    return RegisterFilters(TRUE);
}

STDAPI DllUnregisterServer()
{
    return RegisterFilters(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	for( int i = 0; i < 10; ++i )
		CCamIP::GetName(g_names[i],i);

	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
