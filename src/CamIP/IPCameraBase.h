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

class CIPCameraCallback
{
public:
	CIPCameraCallback() {}
	~CIPCameraCallback() {}
public:
	virtual bool OnImage(void *p,int w,int h) { return false; }
};

class CIPCameraBase
{
public:
	CIPCameraBase() {}
	~CIPCameraBase(void) {}
public:
	virtual void Disconnect(void) = 0;
	virtual bool IsConnected(void) = 0;
	virtual bool IsStarted(void) = 0;
	virtual bool Start(LPCTSTR url=0) = 0;
	virtual void Stop(void) = 0;

	virtual LPCTSTR GetType() { return _T("bmp"); };

	virtual DWORD GetFrameTime() = 0;
	virtual DWORD GetFrameDelay() = 0;

};
