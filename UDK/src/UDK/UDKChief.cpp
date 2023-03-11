/*
 *   ---------------------
 *  |  UDKChief.cpp
 *   ---------------------
 *   This file is part of Unreal DeKompiler.
 *
 *   Unreal DeKompiler is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Unreal DeKompiler is distributed in the hope and belief that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Unreal DeKompiler.  If not, see <https://www.gnu.org/licenses/>.
 *
 *   Timeline:
 *   March, 2023: First Inscription.
 */

#include <wx/filedlg.h>
#include <wx/filesys.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "UDKChief.h"

/** @file
 * @brief UDKApplication code
 *
 * Contains the main() and associated code for UDKApplication.
 */

UDKHalo* UDKApplication::m_Frame = nullptr;

wxIMPLEMENT_APP_CONSOLE(UDKApplication);

bool UDKApplication::OnInit()
{
	m_Frame = new UDKHalo();
	m_Frame->Show(true);
	m_Frame->SetSize(wxDefaultPosition.x, wxDefaultPosition.y, 900, 500);
	return true;
}

UDKHalo::UDKHalo()
	: wxFrame(nullptr, wxID_ANY, "Unreal DeKompiler")
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
		"Help string shown in status bar for this menu item");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_OPEN);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to Unreal DeKompiler!");

	Bind(wxEVT_MENU, &UDKHalo::OnHello, this, ID_Hello);
	Bind(wxEVT_MENU, &UDKHalo::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &UDKHalo::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &UDKHalo::OnOpenFile, this, wxID_OPEN);
}

void UDKHalo::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void UDKHalo::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("Written by The_Cowboy",
		"About Unreal DeKompiler", wxOK | wxICON_INFORMATION);
}

void UDKHalo::OnHello(wxCommandEvent& event)
{
	wxLogMessage("Hello reversal lovers, greetings from Unreal DeKompiler!");
}

void UDKHalo::OnOpenFile(wxCommandEvent& event)
{
	wxFileDialog dialog(this, "Please choose Unreal code package",
							wxEmptyString, wxEmptyString, "*.u", wxFD_OPEN);

	if (dialog.ShowModal() == wxID_OK)
	{
		wxString filename(dialog.GetPath());

		wxFileSystem currentFileSystem;

		wxFSFile* currentFile = currentFileSystem.OpenFile(filename);

		if(filename.find(".u") == 0)
		{
			wxLogError("Sorry, UDK can't and won't work with unfamiliar code packages.");
			return;
		}
	}
}
