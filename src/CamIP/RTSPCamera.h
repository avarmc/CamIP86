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

#include "IPCameraBase.h"

class CRTSPCamera_data;

class CRTSPCamera : public CIPCameraBase
{
protected:
	CRTSPCamera_data *		pData;
	CIPCameraCallback *	pCallback;
	DWORD					tFrame,nFrame;
	WCHAR					urlBase[1024];
	bool					bStop;
	HANDLE					hThread;
	
public:
	CRTSPCamera(CIPCameraCallback *pc);
	~CRTSPCamera(void);
protected:
	static DWORD WINAPI ThreadProc(LPVOID p);
	void ThreadLocal();
	void GetFrames();
	virtual bool Connect(LPCTSTR url=0);
	void SyncDelay(DWORD delay=2000);
public:
	virtual void Disconnect(void);
	virtual bool IsConnected(void);
	virtual bool IsStarted(void);
	virtual bool Start(LPCTSTR url=0);
	virtual void Stop(void);

	virtual LPCTSTR GetType() { return _T("bmp"); };

	virtual DWORD GetFrameTime() { return tFrame; }
	virtual DWORD GetFrameDelay() { return GetTickCount() - tFrame; }

};
