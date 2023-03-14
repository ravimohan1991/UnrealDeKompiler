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
#include <wx/spinbutt.h>

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
class wxAuiNotebook;
class wxCollapsiblePane;
class wxSpinCtrl;

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

	/**
	 * @brief Prepare the manager for various supported features panes
	 *
	 * Do the necessary adding and modification of feature panes
	 *
	 * @see UDKHalo::UDKHalo()
	 */
	//class

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

	/**
	 * @brief Pane for file information
	 *
	 * All the useful information about the .u file package is \n
	 * 
	 * @see InfoPanel
	 */
	InfoPanel* m_FileInfoPanel;

	/**
	 * @brief Container for multiple related tabs
	 *
	 * We use this container for displaying (flashing?) of hex \n
	 * and related data (bytecode basically) which may represent \n
	 * Instructructions or Data, hence IDA
	 */
	wxAuiNotebook* m_IDANotebook;
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
	wxMemoryBuffer m_Buffer;
};

#define ID_DEFAULT wxID_ANY // Default
#define idClose 1000
#define idImportTAGs 1001
#define idExportTAGs 1002
#define idImportTemplate 1003
#define wxID_QUIT 1004
#define idCopyAs 1005
#define idSaveAsDump 1006
#define idFillSelection 1007
#define idInsert 1008
#define idGotoOffset 1009
#define idInterpreter 1010
#define idToolbar 1011
#define idInfoPanel 1012
#define idTagPanel 1013
#define idDisassemblerPanel 1014
#define idSearchPanel 1015
#define idComparePanel 1016
#define idZebraStriping 1017
#define idShowOffset 1018
#define idShowHex 1019
#define idShowText 1020
#define idChecksum 1021
#define idCompare 1022
#define idXORView 1023
#define idHex2Color 1024
#define idDeviceRam 1025
#define idDeviceMemorySPD 1026
#define idProcessRAM 1027
#define idDeviceBackup 1028
#define idDeviceRestore 1029
#define idDeviceErase 1030
#define idFileRO 1031
#define idFileRW 1032
#define idFileDW 1033
#define idDonate 1034
#define idWiki 1035
#define idBugReport 1036
#define wxNEW 1037
#define ID_CHK_UNSIGNED 1038
#define ID_CHK_BIGENDIAN 1039

/// <summary>
/// Base class for DataInterpreter
/// </summary>
class InterpreterGui : public wxPanel
{
protected:
	wxCheckBox* m_CheckUnsigned;
	wxCheckBox* m_CheckBigEndian;
	wxStaticText* m_StaticBin;
	wxTextCtrl* m_TextctrlBinary;
	wxCheckBox* m_CheckEdit;
	wxStaticText* m_StaticAscii;
	wxTextCtrl* m_TextctrlAscii;
	wxStaticText* m_Static8bit;
	wxTextCtrl* m_Textctrl8bit;
	wxStaticText* m_Static16bit;
	wxTextCtrl* m_Textctrl16bit;
	wxStaticText* m_Static32bit;
	wxTextCtrl* m_Textctrl32bit;
	wxStaticText* m_Static64bit;
	wxTextCtrl* m_Textctrl64bit;
	wxStaticText* m_Staticfloat;
	wxTextCtrl* m_Textctrlfloat;
	wxStaticText* m_Staticdouble;
	wxTextCtrl* m_Textctrldouble;
	wxCollapsiblePane* m_CollapsiblePaneTimeMachine;
	wxCheckBox* m_CheckBoxLocal;
	wxPanel* m_PanelTime;
	wxStaticText* m_StaticTimeUTC;
	wxSpinCtrl* m_SpinCtrlTimeUTC;
	wxStaticText* m_StaticTimeUnix;
	wxTextCtrl* m_TextctrlTimeUnix;
	wxStaticText* m_StaticTimeUnix64;
	wxTextCtrl* m_TextctrlTimeUnix64;
	wxStaticText* m_StaticTimeFAT;
	wxTextCtrl* m_TextctrlTimeFAT;
	wxStaticText* m_StaticTimeNTFS;
	wxTextCtrl* m_TextctrlTimeNTFS;
	wxStaticText* m_StaticTimeHFSp;
	wxTextCtrl* m_TextctrlTimeHFSp;
	wxStaticText* m_StaticTimeAPFS;
	wxTextCtrl* m_TextctrlTimeAPFS;
	wxCollapsiblePane* m_CollapsiblePaneExFAT;
	wxStaticText* m_StaticTimeExFATCreation;
	wxTextCtrl* m_TextctrlTimeExFATCreation;
	wxStaticText* m_StaticTimeExFATModification;
	wxTextCtrl* m_TextctrlTimeExFATModification;
	wxStaticText* m_StaticTimeExFATAccess;
	wxTextCtrl* m_TextctrlTimeExFATAccess;

	// Virtual event handlers, overide them in your derived class
	virtual void OnUpdate(wxCommandEvent& event) { event.Skip(); }
	virtual void OnTextEdit(wxKeyEvent& event) { event.Skip(); }
	virtual void OnTextMouse(wxMouseEvent& event) { event.Skip(); }
	virtual void OnCheckEdit(wxCommandEvent& event) { event.Skip(); }
	virtual void OnSpin(wxSpinEvent& event) { event.Skip(); }

public:

	InterpreterGui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxPoint(-1, -1), const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString);
	~InterpreterGui();
};

/// <summary>
/// The rightful interpreter of data
/// </summary>
class DataInterpreter : public InterpreterGui
{
public:
	DataInterpreter(wxWindow* parent, int id = -1, wxPoint pos = wxDefaultPosition, wxSize size = wxSize(-1, -1), int style = wxTAB_TRAVERSAL)
		:InterpreterGui(parent, id, pos, size, style) {
#if wxCHECK_VERSION( 2,9,0 ) && defined( __WXOSX__ )	//onOSX, 8 px too small.
		wxFont f = GetFont();
		f.SetPointSize(10);
		m_textctrl_binary->SetFont(f);
		m_textctrl_ascii->SetFont(f);
		m_textctrl_8bit->SetFont(f);
		m_textctrl_16bit->SetFont(f);
		m_textctrl_32bit->SetFont(f);
		m_textctrl_64bit->SetFont(f);
		m_textctrl_float->SetFont(f);
		m_textctrl_double->SetFont(f);
#endif
#ifdef HAS_A_TIME_MACHINE
		m_collapsiblePane_TimeMachine->Enable(true);
		m_collapsiblePane_TimeMachine->Show(true);
#ifdef HAS_A_EXFAT_TIME
		m_collapsiblePaneExFAT->Enable(true);
		m_collapsiblePaneExFAT->Show(true);
#endif
#endif

		unidata.raw = unidata.mraw = NULL;
	};
	void Set(wxMemoryBuffer buffer);
	void Clear(void);
	void OnUpdate(wxCommandEvent& event);
	void OnSpin(wxSpinEvent& event);
	void OnTextEdit(wxKeyEvent& event);
	void OnTextMouse(wxMouseEvent& event);
	void OnCheckEdit(wxCommandEvent& event);
	wxString AsciiSymbol(unsigned char a);

protected:
	struct unidata
	{
		char* raw, * mraw;	//big endian and little endian
		struct endian {
			int8_t* bit8;
			int16_t* bit16;
			int32_t* bit32;
			int64_t* bit64;
			uint8_t* ubit8;
			uint16_t* ubit16;
			uint32_t* ubit32;
			uint64_t* ubit64;
			float* bitfloat;
			double* bitdouble;
			//_Float128 *bitf128;
			char* raw;
		} little, big;
		short size;
		char* mirrorbfr;
	}unidata;

#ifdef HAS_A_TIME_MACHINE
#pragma pack (1)
	struct FATDate_t {
		unsigned Sec : 5;
		unsigned Min : 6;
		unsigned Hour : 5;
		unsigned Day : 5;
		unsigned Month : 4;
		unsigned Year : 7;
	}FATDate;


	enum TimeFormats { UNIX32, UNIX64, FAT, NTFS, APFS, HFSp, exFAT_C, exFAT_M, exFAT_A, };

	wxString FluxCapacitor(unidata::endian* unit, TimeFormats format);
#endif //HAS_A_TIME_MACHINE
};
