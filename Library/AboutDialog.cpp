/*
  Copyright (C) 2000 Kimmo Pekkola

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "StdAfx.h"
#include "Rainmeter.h"
#include "MeterWindow.h"
#include "Measure.h"
#include "resource.h"
#include "AboutDialog.h"
#include "../revision-number.h"
#include <commctrl.h>

extern CRainmeter* Rainmeter;

INT_PTR CALLBACK AboutProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
HWND g_DialogWin = NULL;
VOID UpdateWidgets(HWND window);

struct PLUGIN_INFO
{
	std::wstring name;
	UINT version;
	std::wstring author;
};
std::vector<PLUGIN_INFO> g_Plugins;

HWND OpenAboutDialog(HWND hwndOwner, HINSTANCE instance)
{
	if (g_DialogWin == NULL)
	{
		g_DialogWin = CreateDialog(instance, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), hwndOwner, AboutProc);
      
		if (g_DialogWin)
		{
			HICON hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_TRAY));
			SendMessage(g_DialogWin, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
	}
	ShowWindow(g_DialogWin, SW_SHOWNORMAL);
	UpdateAboutStatistics();

	return g_DialogWin;
}

void UpdateAboutStatistics()
{
	if (g_DialogWin != NULL && IsWindowVisible(g_DialogWin))
	{
		HWND widget;
		widget = GetDlgItem(g_DialogWin, IDC_ABOUT_ENTRIES);
		int selected = (int)SendMessage(widget, LB_GETCURSEL, NULL, NULL);
		int count = (int)SendMessage(widget, LB_GETCOUNT, NULL, NULL);
		int current = 0;
	
		widget = GetDlgItem(g_DialogWin, IDC_STATISTICS);
		SendMessage(widget, WM_SETREDRAW, 0, 0);

		if (selected == 0)
		{
			int count = ListView_GetItemCount(widget);

			std::list<CRainmeter::LOG_INFO>::const_iterator iter = Rainmeter->m_LogData.begin();
			LVITEM vitem;
			vitem.mask = LVIF_TEXT;

			int i = 0;
			for ( ; iter != Rainmeter->m_LogData.end(); ++iter)
			{
				if (i < count)
				{
					ListView_SetItemText(widget, i, 0, (WCHAR*)(*iter).type.c_str());
				}
				else
				{
					vitem.iItem = i;
					vitem.iSubItem = 0;
					vitem.pszText = (WCHAR*)(*iter).type.c_str();
					ListView_InsertItem(widget, &vitem);
				}
				ListView_SetItemText(widget, i, 1, (WCHAR*)(*iter).timestamp.c_str());
				ListView_SetItemText(widget, i, 2, (WCHAR*)(*iter).message.c_str());

				++i;
			}
		}
		else if (selected == 2)
		{
			widget = GetDlgItem(g_DialogWin, IDC_ABOUT_ENTRIES);
			SendMessage(widget, LB_SETCURSEL, 1, NULL);
			UpdateWidgets(g_DialogWin);
		}
		else if (selected > 2)
		{
			std::map<std::wstring, CMeterWindow*>& windows = Rainmeter->GetAllMeterWindows();

			std::map<std::wstring, CMeterWindow*>::const_iterator iter = windows.begin();
			for( ; iter != windows.end(); ++iter)
			{
				if (current == selected - 3)
				{
					int count = ListView_GetItemCount(widget);

					CMeterWindow* meterWindow = (*iter).second;
					std::list<CMeasure*>& measures = meterWindow->GetMeasures();

					int index = 0;
					std::list<CMeasure*>::const_iterator i = measures.begin();
					for( ; i != measures.end(); ++i)
					{
						const WCHAR* name = (*i)->GetName();
						const WCHAR* val = (*i)->GetStats();

						std::wstring range;
						WCHAR buffer[256];
						double minVal = (*i)->GetMinValue();
						double maxVal = (*i)->GetMaxValue();
						CMeasure::GetScaledValue(1, minVal, buffer);
						range = buffer;
						range += L" - ";
						CMeasure::GetScaledValue(1, maxVal, buffer);
						range += buffer;

						if (name && wcslen(name) > 0)
						{
							if (index < count) 
							{
								ListView_SetItemText(widget, index, 0, (WCHAR*)name);
							}
							else
							{
								LVITEM vitem;
								vitem.mask = LVIF_TEXT;
								vitem.iItem = 0; 
								vitem.iSubItem = 0;
								vitem.pszText = (WCHAR*)name;
								ListView_InsertItem(widget, &vitem);
							}

							if (val && wcslen(val) > 0)
							{
								ListView_SetItemText(widget, index, 1, (WCHAR*)val);
							}
							ListView_SetItemText(widget, index, 2, (WCHAR*)range.c_str());
							++index;
						}
					}

					break;
				}
				++current;
			}
		}
		SendMessage(widget, WM_SETREDRAW, 1, 0);
	}
}

void UpdateWidgets(HWND window) 
{
	HWND widget;
	widget = GetDlgItem(g_DialogWin, IDC_ABOUT_ENTRIES);
	int selected = (int)SendMessage(widget, LB_GETCURSEL, NULL, NULL);
	int count = (int)SendMessage(widget, LB_GETCOUNT, NULL, NULL);

	widget = GetDlgItem(g_DialogWin, IDC_STATISTICS);
	ListView_DeleteAllItems(widget);

	if (selected == 0)
	{
		LVCOLUMN lvc; 
		lvc.mask = LVCF_TEXT; 
		lvc.pszText = L"Log Type";
		ListView_SetColumn(widget, 0, &lvc);
		lvc.pszText = L"Time";
		ListView_SetColumn(widget, 1, &lvc);
		lvc.pszText = L"Message";
		ListView_SetColumn(widget, 2, &lvc);
	}
	else if (selected == 1)
	{
		LVCOLUMN lvc; 
		lvc.mask = LVCF_TEXT; 
		lvc.pszText = L"Plugin";
		ListView_SetColumn(widget, 0, &lvc);
		lvc.pszText = L"Version";
		ListView_SetColumn(widget, 1, &lvc);
		lvc.pszText = L"Author";
		ListView_SetColumn(widget, 2, &lvc);

		// Update the list of plugins
		std::vector<PLUGIN_INFO>::const_iterator iter = g_Plugins.begin();
		LVITEM vitem;
		vitem.mask = LVIF_TEXT;

		int i = 0;
		for ( ; iter != g_Plugins.end(); ++iter)
		{
			if (!(*iter).name.empty())
			{
				vitem.iItem = i;
				vitem.iSubItem = 0;
				vitem.pszText = (WCHAR*)(*iter).name.c_str();
				ListView_InsertItem(widget, &vitem);
			}

			if ((*iter).version != 0)
			{
				WCHAR buffer[256];
				swprintf(buffer, L"%i.%i", (*iter).version / 1000, (*iter).version % 1000);
				ListView_SetItemText(widget, i, 1, buffer);
			}

			ListView_SetItemText(widget, i, 2, (WCHAR*)(*iter).author.c_str());

			++i;
		}

		if (g_Plugins.size() > 0)
		{
			ListView_SetItemState(widget, 0, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
    else
	{
		LVCOLUMN lvc; 
		lvc.mask = LVCF_TEXT; 
		lvc.pszText = L"Measure";
		ListView_SetColumn(widget, 0, &lvc);
		lvc.pszText = L"Value";
		ListView_SetColumn(widget, 1, &lvc);
		lvc.pszText = L"Range";
		ListView_SetColumn(widget, 2, &lvc);
	}
}

typedef LPCTSTR (*GETPLUGINAUTHOR)();
typedef UINT (*GETPLUGINVERSION)();

void ScanPlugins()
{
    WIN32_FIND_DATA fileData;      // Data structure describes the file found
    HANDLE hSearch;                // Search handle returned by FindFirstFile

	std::wstring files = Rainmeter->GetPluginPath() + L"*.dll";

	g_Plugins.clear();

    // Start searching for .ini files in the given directory.
    hSearch = FindFirstFile(files.c_str(), &fileData);
	do
	{
		if(hSearch == INVALID_HANDLE_VALUE) break;    // No more files found

		PLUGIN_INFO info;
		info.name = fileData.cFileName;
		info.version = 0;

		// Try to get the version and author
		std::wstring tmpSz = Rainmeter->GetPluginPath() + fileData.cFileName;
		UINT oldMode = SetErrorMode(0);
		SetErrorMode(oldMode | SEM_FAILCRITICALERRORS);  // Prevent the system from displaying message box
		SetLastError(ERROR_SUCCESS);
		HMODULE dll = LoadLibrary(tmpSz.c_str());
		DWORD err = GetLastError();
		SetErrorMode(oldMode);  // Reset
		if (dll)
		{
			GETPLUGINAUTHOR GetAuthorFunc = (GETPLUGINAUTHOR)GetProcAddress(dll, "GetPluginAuthor");
			if (GetAuthorFunc)
			{
				LPCTSTR author = GetAuthorFunc();
				if (author && wcslen(author) > 0)
				{
					info.author = author;
				}
			}
			
			GETPLUGINVERSION GetVersionFunc = (GETPLUGINVERSION)GetProcAddress(dll, "GetPluginVersion");
			if (GetVersionFunc)
			{
				info.version = GetVersionFunc();
			}
			FreeLibrary(dll);
		}
		else
		{
			DebugLog(L"Unable to load library: \"%s\", ErrorCode=%i", tmpSz.c_str(), err);
		}

		g_Plugins.push_back(info);
	}
	while(FindNextFile(hSearch, &fileData));

    FindClose(hSearch);
}

void RepositionControls(HWND hwndDlg)
{
	RECT r, br, ar, sr, wr;
	HWND widget;

	GetClientRect(hwndDlg, &r);
	GetClientRect(GetDlgItem(hwndDlg, IDOK), &br);
	GetClientRect(GetDlgItem(hwndDlg, IDC_STATIC_ABOUT), &ar);
	GetClientRect(GetDlgItem(hwndDlg, IDC_VERSION_STRING), &sr);

	// Reposition the statistics widgets
	widget = GetDlgItem(hwndDlg, IDC_STATIC_ABOUT);
	SetWindowPos(widget, NULL, 0, 0, r.right - 22, ar.bottom, SWP_NOMOVE | SWP_NOZORDER);
	widget = GetDlgItem(hwndDlg, IDC_VERSION_STRING);
	SetWindowPos(widget, NULL, 0, 0, r.right - 44, sr.bottom, SWP_NOMOVE | SWP_NOZORDER);
	widget = GetDlgItem(hwndDlg, IDC_BUILD_STRING);
	SetWindowPos(widget, NULL, 0, 0, r.right - 44, sr.bottom, SWP_NOMOVE | SWP_NOZORDER);
	widget = GetDlgItem(hwndDlg, IDC_URL_STRING);
	SetWindowPos(widget, NULL, 0, 0, r.right - 44, sr.bottom, SWP_NOMOVE | SWP_NOZORDER);

	widget = GetDlgItem(hwndDlg, IDC_DISABLE_VERSION_CHECK);
	GetClientRect(widget, &wr);
	MapWindowPoints(widget, hwndDlg, (LPPOINT)&wr, 2);
	SetWindowPos(widget, NULL, ((r.right - (wr.right - wr.left)) / 2) + 9, wr.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	widget = GetDlgItem(hwndDlg, IDC_ABOUT_ENTRIES);
	SetWindowPos(widget, NULL, 0, 0, (r.right - 28) / 3, r.bottom - 170, SWP_NOMOVE | SWP_NOZORDER);
	widget = GetDlgItem(hwndDlg, IDC_STATISTICS);
	SetWindowPos(widget, NULL, 18 + ((r.right - 28) / 3), 130, 2 * ((r.right - 28) / 3), r.bottom - 170, SWP_NOZORDER);
	widget = GetDlgItem(hwndDlg, IDOK);
	SetWindowPos(widget, NULL, (r.right - br.right) / 2, r.bottom - br.bottom - 9, br.right, br.bottom, SWP_NOZORDER);
}

BOOL OnInitAboutDialog(HWND window) 
{
	WCHAR tmpSz[256];
	HWND widget;

	widget = GetDlgItem(window, IDC_VERSION_STRING);
	swprintf(tmpSz, L"%s %s%s rev %i %s", APPNAME, APPVERSION, revision_beta ? L" Beta" : L"", revision_number, APPBITS);
	SetWindowText(widget, tmpSz);

	widget = GetDlgItem(window, IDC_BUILD_STRING);
	swprintf(tmpSz, L"Built on %s", ConvertToWide(__DATE__).c_str());
	SetWindowText(widget, tmpSz);

	// Add entries for each config
	widget = GetDlgItem(window, IDC_ABOUT_ENTRIES);
	std::map<std::wstring, CMeterWindow*>& windows = Rainmeter->GetAllMeterWindows();
	std::map<std::wstring, CMeterWindow*>::const_iterator iter = windows.begin();
	int i = 0;
	for( ; iter != windows.end(); ++iter)
	{
		CMeterWindow* meterWindow = (*iter).second;
		wchar_t* skinName = (WCHAR*)meterWindow->GetSkinName().c_str();
		SendMessage(widget, LB_ADDSTRING, NULL, (LPARAM) skinName);
		size_t namelength = wcslen(skinName);

		int currwidth = (INT)SendMessage(widget, LB_GETHORIZONTALEXTENT, NULL, NULL);
		if(6 * namelength > currwidth)
		{
			SendMessage(widget, LB_SETHORIZONTALEXTENT, 6 * namelength, NULL);
		}
	}
	SendMessage(widget, LB_INSERTSTRING, 0, (LPARAM) L"Log");
	SendMessage(widget, LB_INSERTSTRING, 1, (LPARAM) L"Plugins");
	SendMessage(widget, LB_INSERTSTRING, 2, (LPARAM) L"--------------------");

	// Add columns to the list view
	widget = GetDlgItem(window, IDC_STATISTICS);

	//ListView_SetExtendedListViewStyleEx(widget, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(widget, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    LVCOLUMN lvc; 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.iSubItem = 0;
    lvc.pszText = L"Measure";
    lvc.cx = 110;
    lvc.fmt = LVCFMT_LEFT;  // left-aligned column
    ListView_InsertColumn(widget, 0, &lvc);
    lvc.iSubItem = 1;
    lvc.cx = 100;
    lvc.pszText = L"Value";	
    ListView_InsertColumn(widget, 1, &lvc);
    lvc.iSubItem = 1;
    lvc.cx = 150;
    lvc.pszText = L"Range";	
    ListView_InsertColumn(widget, 2, &lvc);

	CheckDlgButton(window, IDC_DISABLE_VERSION_CHECK, Rainmeter->GetDisableVersionCheck() ? BST_CHECKED : BST_UNCHECKED);

	ScanPlugins();
	UpdateWidgets(window);
	RepositionControls(window);

	g_DialogWin = window;

	return TRUE;
}

INT_PTR CALLBACK AboutProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    switch (message) 
    { 
        case WM_INITDIALOG:
			return OnInitAboutDialog(hwndDlg);

		case WM_WINDOWPOSCHANGING:
			{
				WINDOWPOS* pos = (WINDOWPOS*)lParam;

				pos->cx = max(280, pos->cx);
				pos->cy = max(280, pos->cy);
			}
			break;

		case WM_SIZE:
			RepositionControls(hwndDlg);
			break;

		case WM_CLOSE:
			Rainmeter->SaveSettings();
			DestroyWindow(hwndDlg);
			g_DialogWin = NULL;
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_DISABLE_VERSION_CHECK:
				if (IsDlgButtonChecked(hwndDlg, IDC_DISABLE_VERSION_CHECK))
				{
					Rainmeter->SetDisableVersionCheck(TRUE);	
				}
				else
				{
					Rainmeter->SetDisableVersionCheck(FALSE);
				}
				break;

			case IDOK:
				Rainmeter->SaveSettings();
				DestroyWindow(hwndDlg);
				g_DialogWin = NULL;
				return TRUE;

			case IDC_ABOUT_ENTRIES:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					UpdateWidgets(hwndDlg);
					UpdateAboutStatistics();
				} 
				break;
			}
			break;
	}
    return FALSE;
}

