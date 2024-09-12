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

// CamIPcfgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CamIPcfg.h"
#include "CamIPcfgDlg.h"
#include "reg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCamIPcfgDlg dialog




CCamIPcfgDlg::CCamIPcfgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCamIPcfgDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bChanged = false;
	m_cfg.w = m_cfg.h = 0;
	m_cfg.fps = 0;
	m_cfg.enable = false;
	ZeroMemory(&m_cfg.rs,sizeof(m_cfg.rs));
}

void CCamIPcfgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CAM, m_list);
	DDX_Text(pDX, IDC_EDIT_URL, m_cfg.url);
	DDX_Text(pDX, IDC_EDIT_NAME, m_cfg.name);
	DDX_Text(pDX, IDC_EDIT_FPS, m_cfg.fps);
	DDX_Text(pDX, IDC_EDIT_TSHIFT, m_cfg.tShift);
	DDX_Control(pDX, IDC_BUTTON_APPLY, m_apply);
	DDX_Text(pDX, IDC_EDIT_W, m_cfg.w);
	DDX_Text(pDX, IDC_EDIT_H, m_cfg.h);
	DDX_Text(pDX, IDC_EDIT_TOP, m_cfg.rs.top);
	DDX_Text(pDX, IDC_EDIT_BOTTOM, m_cfg.rs.bottom);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_cfg.rs.left);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_cfg.rs.right);
	DDX_Check(pDX, IDC_CHECK_ENABLE, m_cfg.enable);
	DDX_Control(pDX, IDC_CHECK_ENABLE, m_enable);
}

BEGIN_MESSAGE_MAP(CCamIPcfgDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CCamIPcfgDlg::OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LIST_CAM, &CCamIPcfgDlg::OnLbnSelchangeListCam)
	ON_EN_CHANGE(IDC_EDIT_URL, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_NAME, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_W, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_H, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_TOP, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_BOTTOM, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_LEFT, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_RIGHT, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_FPS, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_TSHIFT, &CCamIPcfgDlg::OnEnChangeEdit)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CCamIPcfgDlg::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_CHECK_ENABLE, &CCamIPcfgDlg::OnBnClickedCheckEnable)
END_MESSAGE_MAP()


// CCamIPcfgDlg message handlers

BOOL CCamIPcfgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	for( int i = 0; i < 10; ++i )
	{
		CString name;
		name.Format(_T("CamIP %d"),i+1);
		m_list.AddString(name);
	}

	PreLoadSel();

	CheckEnable();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCamIPcfgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCamIPcfgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCamIPcfgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCamIPcfgDlg::OnBnClickedCancel()
{
	UpdateData();

	CheckApply();

	SaveSel();

	OnCancel();
}

void CCamIPcfgDlg::OnLbnSelchangeListCam()
{
	UpdateData();

	int idSelected = m_list.GetCurSel();
	if(m_cfg.id == idSelected)
		return;
	
	CheckApply();
	
	m_apply.EnableWindow(FALSE);

	LoadData(idSelected);
	
	CheckEnable();
}

void CCamIPcfgDlg::OnEnChangeEdit()
{
	UpdateData();

	m_bChanged = true;
	CheckEnable();
}

void CCamIPcfgDlg::OnBnClickedCheckEnable()
{
	UpdateData();

	m_bChanged = true;
	CheckEnable();
}

void CCamIPcfgDlg::OnBnClickedButtonApply()
{
	LPCWSTR file = L"CamIP.ax";
	LPCWSTR rs32 = L"regsvr32";
	
	CString path0 = CCamIPcfgApp::GetDefaultPath();
	UpdateData();

	BeginWaitCursor();

	CString param;

	param.Format(_T("/u /s %s"),file);
	ShellExecute(m_hWnd,NULL,rs32,param,path0,SW_SHOW);

	SaveData();

	param.Format(_T("/s %s"),file);
	ShellExecute(m_hWnd,NULL,rs32,param,path0,SW_SHOW);


	m_bChanged = false;
	CheckEnable();

	EndWaitCursor();
}

void CCamIPcfgDlg::CheckEnable(void)
{
	HWND hWnd = m_list.m_hWnd;
	BOOL bEnable = (m_cfg.id >= 0);

	while( (hWnd = ::FindWindowEx(m_hWnd,hWnd,0,0)) != m_list.m_hWnd )
	{
		if( hWnd == m_enable.m_hWnd )
			::EnableWindow( hWnd, bEnable );
		else
		if( hWnd == m_apply.m_hWnd )
			::EnableWindow( hWnd, m_bChanged );
		else
			::EnableWindow( hWnd, bEnable && m_cfg.enable );
	}
}

void CCamIPcfgDlg::CheckApply(void)
{
	if(!m_bChanged || m_cfg.id < 0) 
		return;

	CString strMsg,strTitle;
	strMsg.LoadString(IDS_STR_APPLY_MSG);
	strTitle.LoadString(IDS_STR_APPLY_TITLE);

	if( MessageBox(strMsg,strTitle,MB_YESNO|MB_ICONQUESTION) == IDYES )
		OnBnClickedButtonApply();

	m_bChanged = false;
}

void CCamIPcfgDlg::LoadData(int id)
{
	WCHAR reg[256],name[256];

	_stprintf_s(reg,256,L"SOFTWARE\\ELSYS\\CamIP\\%0.2u",id);
	_stprintf_s(name,256,L"CamIP %0.2u",id+1);

	m_cfg.id = id;
	m_cfg.enable = GetProfileIntCmd(reg,L"enable",0);
	m_cfg.fps = GetProfileFloatCmd(reg,L"fps",25);
	m_cfg.w = GetProfileIntCmd(reg,L"w",640);
	m_cfg.h = GetProfileIntCmd(reg,L"h",480);

	m_cfg.tShift = GetProfileIntCmd(reg,L"tShift",0);

	m_cfg.rs.left = GetProfileIntCmd(reg,L"crop_l",0);
	m_cfg.rs.right = GetProfileIntCmd(reg,L"crop_r",0);
	m_cfg.rs.top = GetProfileIntCmd(reg,L"crop_t",0);
	m_cfg.rs.bottom = GetProfileIntCmd(reg,L"crop_b",0);

	if(id == 0)
	{
		m_cfg.url = GetProfileStringCmd(reg,L"url",L"rtsp://root:12345@192.168.1.201/axis-media/media.amp");
		m_cfg.name = GetProfileStringCmd(reg,L"name",L"CamIP RTSP Example");
	} else
	if(id == 1)
	{
		m_cfg.url = GetProfileStringCmd(reg,L"url",L"http://root:12345@192.168.1.201/axis-cgi/mjpg/video.cgi");
		m_cfg.name = GetProfileStringCmd(reg,L"name",L"CamIP MJPEG Example");
	} else
	{
		m_cfg.url = GetProfileStringCmd(reg,L"url",L"");
		m_cfg.name = GetProfileStringCmd(reg,L"name",name);
	} 

	UpdateData(FALSE);
}

void CCamIPcfgDlg::SaveData()
{
	// rtsp://root:12345@192.168.1.201/axis-media/media.amp
	WCHAR reg[256];

	if(m_cfg.id < 0)
		return;

	_stprintf(reg,L"SOFTWARE\\ELSYS\\CamIP\\%0.2u",m_cfg.id);

	WriteProfileIntCmd(reg,L"enable",m_cfg.enable);
	WriteProfileFloatCmd(reg,L"fps",m_cfg.fps);
	WriteProfileIntCmd(reg,L"w",m_cfg.w);
	WriteProfileIntCmd(reg,L"h",m_cfg.h);

	WriteProfileIntCmd(reg,L"tShift",m_cfg.tShift);

	WriteProfileIntCmd(reg,L"crop_l",m_cfg.rs.left);
	WriteProfileIntCmd(reg,L"crop_r",m_cfg.rs.right);
	WriteProfileIntCmd(reg,L"crop_t",m_cfg.rs.top);
	WriteProfileIntCmd(reg,L"crop_b",m_cfg.rs.bottom);

	WriteProfileStringCmd(reg,L"url",m_cfg.url);
	WriteProfileStringCmd(reg,L"name",m_cfg.name);

}


void CCamIPcfgDlg::PreLoadSel(void)
{
	WCHAR reg[256] = L"SOFTWARE\\ELSYS\\CamIP";

	int sel = GetProfileIntCmd(reg,L"sel",-1);
	m_list.SetCurSel(sel);

	OnLbnSelchangeListCam();
}

void CCamIPcfgDlg::SaveSel(void)
{
	WCHAR reg[256] = L"SOFTWARE\\ELSYS\\CamIP";

	WriteProfileIntCmd(reg,L"sel",m_list.GetCurSel());
}
