// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003 - Tim Kemp and Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"
#include "TortoiseProc.h"
#include "SetProxyPage.h"


// CSetProxyPage dialog

IMPLEMENT_DYNAMIC(CSetProxyPage, CPropertyPage)
CSetProxyPage::CSetProxyPage()
	: CPropertyPage(CSetProxyPage::IDD)
	, m_serveraddress(_T(""))
	, m_serverport(0)
	, m_username(_T(""))
	, m_password(_T(""))
	, m_timeout(0)
	, m_isEnabled(FALSE)
{
	this->m_pPSP->dwFlags &= ~PSP_HASHELP;
}

CSetProxyPage::~CSetProxyPage()
{
}

void CSetProxyPage::SaveData()
{
	if (m_isEnabled)
	{
		CString temp;
		m_regServeraddress = m_serveraddress;
		temp.Format(_T("%d"), m_serverport);
		m_regServerport = temp;
		m_regUsername = m_username;
		m_regPassword = m_password;
		temp.Format(_T("%d"), m_timeout);
		m_regTimeout = temp;
	} // if (m_isEnabled)
	else
	{
		m_regServeraddress.removeValue();
		m_regServerport.removeValue();
		m_regUsername.removeValue();
		m_regPassword.removeValue();
		m_regTimeout.removeValue();
	}
}

void CSetProxyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SERVERADDRESS, m_serveraddress);
	DDX_Text(pDX, IDC_SERVERPORT, m_serverport);
	DDX_Text(pDX, IDC_USERNAME, m_username);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_Text(pDX, IDC_TIMEOUT, m_timeout);
	DDX_Check(pDX, IDC_ENABLE, m_isEnabled);
}


BEGIN_MESSAGE_MAP(CSetProxyPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ENABLE, OnBnClickedEnable)
	ON_EN_CHANGE(IDC_SERVERADDRESS, OnEnChangeServeraddress)
	ON_EN_CHANGE(IDC_SERVERPORT, OnEnChangeServerport)
	ON_EN_CHANGE(IDC_USERNAME, OnEnChangeUsername)
	ON_EN_CHANGE(IDC_PASSWORD, OnEnChangePassword)
	ON_EN_CHANGE(IDC_TIMEOUT, OnEnChangeTimeout)
END_MESSAGE_MAP()



BOOL CSetProxyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_tooltips.Create(this);
	m_tooltips.AddTool(IDC_SERVERADDRESS, IDS_SETTINGS_PROXYSERVER_TT);
	m_tooltips.SetEffectBk(CBalloon::BALLOON_EFFECT_HGRADIENT);
	m_tooltips.SetGradientColors(0x80ffff, 0x000000, 0xffff80);

	m_regServeraddress = CRegString(_T("Software\\Tigris.org\\Subversion\\Servers\\global\\http-proxy-host"), _T(""), 0, HKEY_LOCAL_MACHINE);
	m_regServerport = CRegString(_T("Software\\Tigris.org\\Subversion\\Servers\\global\\http-proxy-port"), _T(""), 0, HKEY_LOCAL_MACHINE);
	m_regUsername = CRegString(_T("Software\\Tigris.org\\Subversion\\Servers\\global\\http-proxy-username"), _T(""), 0, HKEY_LOCAL_MACHINE);
	m_regPassword = CRegString(_T("Software\\Tigris.org\\Subversion\\Servers\\global\\http-proxy-password"), _T(""), 0, HKEY_LOCAL_MACHINE);
	m_regTimeout = CRegString(_T("Software\\Tigris.org\\Subversion\\Servers\\global\\http-proxy-timeout"), _T(""), 0, HKEY_LOCAL_MACHINE);
	
	m_serveraddress = m_regServeraddress;
	m_serverport = _ttoi((LPCTSTR)(CString)m_regServerport);
	m_username = m_regUsername;
	m_password = m_regPassword;
	m_timeout = _ttoi((LPCTSTR)(CString)m_regTimeout);

	if (m_serveraddress.IsEmpty())
	{
		m_isEnabled = FALSE;
		EnableGroup(FALSE);
		//now since we already created our registry entries
		//we delete them here again...
		m_regServeraddress.removeValue();
		m_regServerport.removeValue();
		m_regUsername.removeValue();
		m_regPassword.removeValue();
		m_regTimeout.removeValue();
	}
	else
	{
		m_isEnabled = TRUE;
		EnableGroup(TRUE);
	}
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSetProxyPage::OnBnClickedEnable()
{
	UpdateData();
	if (m_isEnabled)
	{
		EnableGroup(TRUE);
	}
	else
	{
		EnableGroup(FALSE);
	}
}

void CSetProxyPage::EnableGroup(BOOL b)
{
	GetDlgItem(IDC_SERVERADDRESS)->EnableWindow(b);
	GetDlgItem(IDC_SERVERPORT)->EnableWindow(b);
	GetDlgItem(IDC_USERNAME)->EnableWindow(b);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(b);
	GetDlgItem(IDC_TIMEOUT)->EnableWindow(b);
}



BOOL CSetProxyPage::PreTranslateMessage(MSG* pMsg)
{
	m_tooltips.RelayEvent(pMsg);
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CSetProxyPage::OnEnChangeServeraddress()
{
	SetModified();
}

void CSetProxyPage::OnEnChangeServerport()
{
	SetModified();
}

void CSetProxyPage::OnEnChangeUsername()
{
	SetModified();
}

void CSetProxyPage::OnEnChangePassword()
{
	SetModified();
}

void CSetProxyPage::OnEnChangeTimeout()
{
	SetModified();
}

BOOL CSetProxyPage::OnApply()
{
	SaveData();
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}
