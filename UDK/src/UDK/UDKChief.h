/*
 *   ---------------------
 *  |  UDKChief.h
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
 * @brief UDKApplication code header file
 *
 * Contains the main() and associated code for UDKApplication.
 */

class UDKHalo;

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

	/**
	 * @brief Obtain pointer to main frame
	 *
	 * This routine returns the pointer to main frame or parent to all
	 *
	 * @return UDKHalo pointer to main window
	 */
	static UDKHalo* GetMainFrame() { return m_Frame; }
private:
	/**
	 * @brief Pointer to main frame
	 *
	 * This is the pointer to UDK's main window. \n
	 * We are not using smart pointer because lifetime is taken care of \n
	 * and it is just an extra complexity we can do without.
	 */
	static UDKHalo* m_Frame;
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
	 * @see UDKHalo::OnExit(wxCommandEvent& event)
	 * @see UDKHalo::OnAbout(wxCommandEvent& event)
	 */
	UDKHalo();

private:
	/**
	 * @brief Hello entry callback
	 *
	 * Opens up a modal message window with desired hello message.
	 *
	 * @see UDKHalo::MyFrame()
	 */
	void OnHello(wxCommandEvent& event);

	/**
	 * @brief Exit entry callback
	 *
	 * Closes the application.
	 *
	 * @see UDKHalo::MyFrame()
	 */
	void OnExit(wxCommandEvent& event);

	/**
	 * @brief About entry callback
	 *
	 * Opens up a modal message window with desired application information.
	 *
	 * @see UDKHalo::MyFrame()
	 */
	void OnAbout(wxCommandEvent& event);

	/**
	 * @brief OpenFile entry callback
	 *
	 * Opens up OpenFile dialog window 
	 *
	 * @see UDKHalo::MyFrame()
	 */
	void OnOpenFile(wxCommandEvent& event);
};

enum
{
	ID_Hello = 1
};
