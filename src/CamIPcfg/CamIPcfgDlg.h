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

// CamIPcfgDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CCamIPcfgDlg dialog
class CCamIPcfgDlg : public CDialog
{
// Construction
public:
	CCamIPcfgDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CAMIPCFG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	typedef struct tagCAM_CONFIG
	{
		int		id;
		int		enable;
		CString	url;
		CString	name;
		float	fps;
		int		w,h;			// dst image size
		RECT	rs;				// crop rect
		int		tShift;
	}CAM_CONFIG;

	CListBox	m_list;
	CButton		m_apply;
	CButton		m_enable;

	CAM_CONFIG	m_cfg;

	bool		m_bChanged;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeListCam();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedCheckEnable();

	void CheckEnable(void);
	void CheckApply(void);
	void LoadData(int id);
	void SaveData();
	void PreLoadSel(void);
	void SaveSel(void);

	DECLARE_MESSAGE_MAP()
};
