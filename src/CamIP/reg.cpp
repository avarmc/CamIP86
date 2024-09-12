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
#include "reg.h"

int GetProfileStringCmd(const WCHAR* szPath,const WCHAR* tag, WCHAR *str, size_t str_max)
{
	HKEY hKey;

	*str = 0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0,  KEY_READ, &hKey) != ERROR_SUCCESS)
		return -1;


	DWORD size = str_max;
	DWORD type = REG_SZ;
	LONG ret = RegQueryValueEx(hKey,tag,0,&type,(LPBYTE)str,&size);

	if(ret != ERROR_SUCCESS)
		return -1;

	if(RegCloseKey(hKey) != ERROR_SUCCESS)
		return -1;
	

	return size;
}

#if _MFC_VER >= 1400 
CString GetProfileStringCmd(const WCHAR* szPath,const WCHAR* tag,const WCHAR *str)
{
	WCHAR tmp[2048] = L"";
	if(GetProfileStringCmd(szPath,tag,tmp,2048) >= 0)
		return tmp;
	return str;
}
#endif

int WriteProfileStringCmd(const WCHAR* szPath,const WCHAR* tag,const  WCHAR* v)
{

	HKEY hKey;
	DWORD dwDisposition;

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS){
		return 1;
	}
	
	if (dwDisposition != REG_CREATED_NEW_KEY && dwDisposition != REG_OPENED_EXISTING_KEY)
		return 2;

	if(RegSetValueEx(hKey, tag, 0, REG_SZ, (BYTE*)v, (_tcslen(v)+1)*sizeof(WCHAR)) != ERROR_SUCCESS){
		return 3;
	}

	if(RegCloseKey(hKey) != ERROR_SUCCESS){
		return 4;
	}
	return 0;
}


int GetProfileIntCmd(const WCHAR* szPath,const WCHAR* tag, int v)
{
	HKEY hKey;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0,  KEY_READ, &hKey) != ERROR_SUCCESS)
		return v;


	DWORD size = sizeof(int);
	DWORD type = REG_DWORD;
	int r = v;
	LONG ret = RegQueryValueEx(hKey,tag,0,&type,(LPBYTE)&r,&size);

	if(ret != ERROR_SUCCESS)
		return v;

	if(RegCloseKey(hKey) != ERROR_SUCCESS)
		return v;
	

	return r;
}


int WriteProfileIntCmd(const WCHAR* szPath,const WCHAR* tag,int v)
{

	HKEY hKey;
	DWORD dwDisposition;

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS){
		return 1;
	}
	
	if (dwDisposition != REG_CREATED_NEW_KEY && dwDisposition != REG_OPENED_EXISTING_KEY)
		return 2;

	if(RegSetValueEx(hKey, tag, 0, REG_DWORD, (BYTE*)&v, sizeof(v)) != ERROR_SUCCESS){
		return 3;
	}

	if(RegCloseKey(hKey) != ERROR_SUCCESS){
		return 4;
	}
	return 0;
}


float GetProfileFloatCmd(const WCHAR* szPath,const WCHAR* tag, float v)
{
	WCHAR str[64];
	if( GetProfileStringCmd(szPath,tag,str,64) < 0)
		return v;
	return (float)_wtof(str);
}

int WriteProfileFloatCmd(const WCHAR* szPath,const WCHAR* tag, float v)
{
	WCHAR str[64];
	_stprintf_s(str,64,L"%g",v);
	return WriteProfileStringCmd(szPath,tag,str);
}