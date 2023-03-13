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

#pragma once

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

/** @file
 * @brief UDKApplication code header file
 *
 * Contains the main() and associated code for UDKApplication.
 */

// Forward declarations
class UDKHalo;
class wxAuiManager;
class InfoPanel;
class DisassemblerPanel;

/// <summary>
/// The class corresponding to the main UDK application instance
/// </summary>
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

/// <summary>
/// The UDK main window frame class
/// </summary>
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
	 * @see UDKHalo::UDKHalo()
	 */
	void OnHello(wxCommandEvent& event);

	/**
	 * @brief Exit entry callback
	 *
	 * Closes the application.
	 *
	 * @see UDKHalo::UDKHalo()
	 */
	void OnExit(wxCommandEvent& event);

	/**
	 * @brief About entry callback
	 *
	 * Opens up a modal message window with desired application information.
	 *
	 * @see UDKHalo::UDKHalo()
	 */
	void OnAbout(wxCommandEvent& event);

	/**
	 * @brief OpenFile entry callback
	 *
	 * Opens up OpenFile dialog window 
	 *
	 * @see UDKHalo::UDKHalo()
	 */
	void OnOpenFile(wxCommandEvent& event);

	/**
	 * @brief Prepare the manager for various supported features panes
	 *
	 * Do the necessary adding and modification of feature panes
	 *
	 * @see UDKHalo::UDKHalo()
	 */
	void PrepareAUI(void);

private:
	/**
	 * @brief For managing varitey of panes or panels
	 *
	 * Chiefly used for adding, handling resizing, docking, and more \n
	 * of utility panes (to be) present in UDKHalo.\n
	 *
	 * wxAuiManager works as follows: the programmer adds panes to the \n
	 * class, or makes changes to existing pane properties (dock \n
	 * position, floating state, show state, etc.). To apply these \n
	 * changes, wxAuiManager's Update() function is called. This batch \n 
	 * processing can be used to avoid flicker, by modifying more than \n 
	 * one pane at a time, and then "committing" all of the changes at \n
	 * once by calling Update().
	 */
	wxAuiManager* m_PaneManager;

	/**
	 * @brief Disassembler pane
	 *
	 * The lookand(partial)feel of UDK's disassembler pane 
	 */
	DisassemblerPanel* m_DisassemblerPanel;

	InfoPanel* m_FileInfoPanel;
};

enum
{
	ID_Hello = 1
};

/// <summary>
/// Super class of InfoPanel
/// </summary>
class InfoPanelGui : public wxPanel
{
private:

protected:
	wxStaticText* m_InfoPanelText;

public:

	InfoPanelGui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(140, 111), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString);
	~InfoPanelGui();

};

/// <summary>
/// Class which displays relevant information of a file \n
/// including name, path, size, and access
/// </summary>
class InfoPanel : public InfoPanelGui
{
public:
	InfoPanel(wxWindow* parent, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize(-1, -1), int style = wxTAB_TRAVERSAL)
		:InfoPanelGui(parent, id, pos, size, style) {}
	void Set(wxFileName flnm, uint64_t lenght, wxString AccessMode, int FD, wxString XORKey);
	void OnUpdate(wxCommandEvent& event) {}
};

/// <summary>
/// Super class of DisassemblerPanel
/// </summary>
class DisassemblerPanelGUI : public wxPanel
{
private:

protected:
	wxChoice* m_ChoiceVendor;
	wxChoice* m_ChoiceASMType;
	wxChoice* m_ChoiceBitMode;
	wxTextCtrl* m_DasmCtrl;

	// Virtual event handlers, overide them in your derived class
	virtual void OnUpdate(wxCommandEvent& event) { event.Skip(); }


public:

	DisassemblerPanelGUI(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(273, 310), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString);
	~DisassemblerPanelGUI();

};

/// <summary>
/// Class whose instance is the disassembler panel
/// </summary>
class DisassemblerPanel : public DisassemblerPanelGUI
{
public:
	DisassemblerPanel(UDKHalo* parent_, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize(-1, -1), int style = wxTAB_TRAVERSAL)
		:DisassemblerPanelGUI((wxWindow*)parent_, id) {};
	void Set(wxMemoryBuffer buffer);
	void OnUpdate(wxCommandEvent& event);
	void Clear(void);
	wxMemoryBuffer mybuff;
};