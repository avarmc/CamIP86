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

#include <process.h>

#if _MFC_VER >= 1400 
CString GetProfileStringCmd(const WCHAR* szPath,const WCHAR* tag,const WCHAR *str);
#endif
int GetProfileStringCmd(const WCHAR* szPath,const WCHAR* tag, WCHAR *str, size_t str_max);
int GetProfileIntCmd(const WCHAR* szPath,const WCHAR* tag, int v);
float GetProfileFloatCmd(const WCHAR* szPath,const WCHAR* tag, float v);
int WriteProfileStringCmd(const WCHAR* szPath,const WCHAR* tag,const  WCHAR* v);
int WriteProfileIntCmd(const WCHAR* szPath,const WCHAR* tag, int v);
int WriteProfileFloatCmd(const WCHAR* szPath,const WCHAR* tag, float v);
