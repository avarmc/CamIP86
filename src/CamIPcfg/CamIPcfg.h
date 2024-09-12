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

// CamIPcfg.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCamIPcfgApp:
// See CamIPcfg.cpp for the implementation of this class
//

class CCamIPcfgApp : public CWinApp
{
public:
	CCamIPcfgApp();

// Overrides
public:
	virtual BOOL InitInstance();

	static CString GetDefaultPath();
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCamIPcfgApp theApp;