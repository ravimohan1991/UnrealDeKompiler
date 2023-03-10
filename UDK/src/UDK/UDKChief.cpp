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

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

/** @file
 * @brief UDKApplication code
 *
 * Contains the main() and associated code for UDKApplication.
 */

class UDKApplication : public wxApp
{
public:
	/**
	 * @brief Create new frame and display that
	 *
	 * This routine creates new frame and displays the same.
	 *
	 * @return true on successful initialization of "hello world" application
	 * @see wxTopLevelWindowMSW::Show()
	 */
	virtual bool OnInit();
};

class UDKHalo : public wxFrame
{
public:
	/**
	 * @brief Frame setup
	 *
	 * Sets and populates menubar with File and Help menus. File contains Hello and \n
	 * Quit entries while Help contains About entry. A status bar at the bottom is \n
	 * also created. A relevant bind to the OnHello, OnExit, and OnAbout routines \n
	 * is done for desired callback.
	 *
	 * @see MyFrame::OnHello(wxCommandEvent&)
	 * @see MyFrame::OnExit()
	 * @see MyFrame::OnAbout(wxCommandEvent& event)
	 */
	UDKHalo();

private:
	/**
	 * @brief Hello entry callback
	 *
	 * Opens up a modal message window with desired hello message.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnHello(wxCommandEvent& event);

	/**
	 * @brief Exit entry callback
	 *
	 * Closes the application.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnExit(wxCommandEvent& event);

	/**
	 * @brief About entry callback
	 *
	 * Opens up a modal message window with desired application information.
	 *
	 * @see MyFrame::MyFrame()
	 */
	void OnAbout(wxCommandEvent& event);
};

enum
{
	ID_Hello = 1
};

wxIMPLEMENT_APP_CONSOLE(UDKApplication);

bool UDKApplication::OnInit()
{
	UDKHalo* frame = new UDKHalo();
	frame->Show(true);
	frame->SetSize(wxDefaultPosition.x, wxDefaultPosition.y, 900, 500);

	return true;
}

UDKHalo::UDKHalo()
	: wxFrame(nullptr, wxID_ANY, "Unreal DeKompiler")
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
		"Help string shown in status bar for this menu item");
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
