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

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "CrashHandler.h"

#include "resource.h"		// main symbols
#include "Utils.h"
#include "BrowseFolder.h"
#include "ProgressDlg.h"
#include "CmdLineParser.h"
#include "AboutDlg.h"
#include "LogDlg.h"
#include "SVNProgressDlg.h"
#include "CheckoutDlg.h"
#include "LogPromptDlg.h"
#include "WatcherDlg.h"
#include "ImportDlg.h"
#include "SwitchDlg.h"
#include "MergeDlg.h"
#include "CopyDlg.h"
#include "Settings.h"
#include "RenameDlg.h"
#include "UpdateDlg.h"
#include "AddDlg.h"

/**
 * \ingroup TortoiseProc
 * Main class of the TortoiseProc.exe\n
 * It is the entry point when calling the TortoiseProc.exe and
 * handles the command line. Depending on the command line
 * other 'modules' are called, usually dialog boxes which 
 * themselves then execute a specific function.\n\n
 * Many commands are executed using the CSVNProgressDlg which
 * just displays the common notify callbacks of the Subversion commands.
 *
 * \par requirements
 * win95 or later
 * winNT4 or later
 * MFC
 *
 * \version 1.0
 * first version
 *
 * \date 10-20-2002
 *
 * \author kueng
 *
 * \par license
 * This code is absolutely free to use and modify. The code is provided "as is" with
 * no expressed or implied warranty. The author accepts no liability if it causes
 * any damage to your computer, causes your pet to fall ill, increases baldness
 * or makes your car start emitting strange noises when you start it up.
 * This code has no bugs, just undocumented features!
 * 
 * \todo fill in the missing commands
 *
 * \bug 
 *
 */
class CTortoiseProcApp : public CWinApp
{
public:
	CTortoiseProcApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
private:
	HANDLE	m_mutex;
};

extern CTortoiseProcApp theApp;