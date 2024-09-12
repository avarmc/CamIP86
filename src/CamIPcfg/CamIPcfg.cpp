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
 * License along with viamcap_rtsp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */ 

// CamIPcfg.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CamIPcfg.h"
#include "CamIPcfgDlg.h"
#include "reg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCamIPcfgApp

BEGIN_MESSAGE_MAP(CCamIPcfgApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCamIPcfgApp construction

CCamIPcfgApp::CCamIPcfgApp()
{
}


// The one and only CCamIPcfgApp object

CCamIPcfgApp theApp;


// CCamIPcfgApp initialization

BOOL CCamIPcfgApp::InitInstance()
{
	CoInitialize(0);

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if( CString( m_lpCmdLine ) == _T("/install") )
	{
	    TCHAR path[MAX_PATH];

		if(  GetModuleFileName(m_hInstance, path, sizeof(path)) > 0 ) 
		{
			WriteProfileStringCmd(L"SOFTWARE\\ELSYS\\CamIP",_T("path"),path);
		}

		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("ELSYS\\CamIP"));

	CCamIPcfgDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


CString CCamIPcfgApp::GetDefaultPath(void)
{
	CString path,hlpPath;

	hlpPath = AfxGetApp()->m_pszHelpFilePath;
	if(hlpPath.ReverseFind('\\') != -1)
	{
		path = hlpPath.Left(hlpPath.ReverseFind('\\')+1);
	}

	return path;
}
