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
#include <wx/aui/framemanager.h>
#include <wx/aui/auibook.h>
#include <types.h>
#include <extern.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "UDKChief.h"
#include "UDKControl.h"

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

	return true;
}

UDKHalo::UDKHalo()
	: wxFrame(nullptr, wxID_ANY, "Unreal DeKompiler", wxDefaultPosition, wxSize(1400, 800), wxDEFAULT_FRAME_STYLE)
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

	// Instantiate a notebook
	m_IDANotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT | wxAUI_NB_WINDOWLIST_BUTTON);

	// Set panes and panels
	PrepareAUI();

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

void UDKHalo::PrepareAUI(void)
{
	m_PaneManager = new wxAuiManager(this);
	m_PaneManager->SetManagedWindow(this);

	m_PaneManager->AddPane(m_IDANotebook, wxAuiPaneInfo().
		CaptionVisible(false).
		Name(wxT("Notebook")).
		Caption(wxT("Notebook")).
		MinSize(wxSize(150, 100)).
		CloseButton(false).
		Center().Layer(1));

	m_DisassemblerPanel = new DisassemblerPanel(this, -1);
	m_PaneManager->AddPane(m_DisassemblerPanel, wxAuiPaneInfo().
		Name(_("Disassembler Panel")).
		Caption(_("The Disassembler")).
		TopDockable(true).
		BottomDockable(true).
		CloseButton(true).
		MinSize(wxSize(180, 200)).
		BestSize(wxSize(240, 200)).
		Show(true).
		Right().Layer(1));

	m_FileInfoPanel = new InfoPanel(this, -1);
	m_PaneManager->AddPane(m_FileInfoPanel, wxAuiPaneInfo().
		Name(_("InfoPanel")).
		Caption(_("Information Panel")).
		TopDockable(false).
		CloseButton(true).
		BottomDockable(false).
		BestSize(wxSize(140, 140)).
		Show(true).
		Resizable(false).
		Left().Layer(1).Position(1));

	wxString tempStr;
	MyConfigBase::Get()->Read(_T("LastPerspective"), &tempStr, wxEmptyString);
	m_PaneManager->LoadPerspective(tempStr);
	m_PaneManager->Update();
}

InfoPanelGui::InfoPanelGui(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer(wxVERTICAL);

	m_InfoPanelText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	m_InfoPanelText->Wrap(-1);
	mainSizer->Add(m_InfoPanelText, 0, wxALL, 2);


	this->SetSizer(mainSizer);
	this->Layout();
}

InfoPanelGui::~InfoPanelGui()
{
}

void InfoPanel::Set(wxFileName flnm, uint64_t lenght, wxString AccessMode, int FD, wxString XORKey)
{
	static wxMutex mutexinfo;
	mutexinfo.Lock();

	struct stat* sbufptr = new struct stat;
	fstat(FD, sbufptr);

	wxString info_string;
	info_string = info_string + _("Name:") + wxT("\t") + flnm.GetFullName() + wxT("\n") +
		_("Path:") + wxT("\t") + flnm.GetPath() + wxT("\n") +
		_("Size:") + wxT("\t") + wxFileName::GetHumanReadableSize(wxULongLong(lenght)) + wxT("\n") +
		_("Access:") + wxT("\t") + AccessMode + wxT("\n") +
		_("Device:") + wxT("\t") +
#ifdef __WXMSW__
		(sbufptr->st_mode == 0 ? _("BLOCK") : _("FILE"))
#else
		(S_ISREG(sbufptr->st_mode) ? _("FILE") :
			S_ISDIR(sbufptr->st_mode) ? _("DIRECTORY") :
			S_ISCHR(sbufptr->st_mode) ? _("CHARACTER") :
			S_ISBLK(sbufptr->st_mode) ? _("BLOCK") :
			S_ISFIFO(sbufptr->st_mode) ? _("FIFO") :
			S_ISLNK(sbufptr->st_mode) ? _("LINK") :
			S_ISSOCK(sbufptr->st_mode) ? _("SOCKET") :
			wxT("?")
			)
#endif
		+ wxT("\n");
#ifdef __WXMSW__
	if (sbufptr->st_mode == 0)	//Enable block size detection code on windows targets,
#else
	if (S_ISBLK(sbufptr->st_mode))
#endif
	{
		info_string += _("Sector Size: ") + wxString::Format(wxT("%u\n"), 0);//FDtoBlockSize(FD)); <----------- Needs to be written
		info_string += _("Sector Count: ") + wxString::Format("%" wxLongLongFmtSpec "u\n", 0);//FDtoBlockCount(FD));
	}

	if (XORKey != wxEmptyString)
		info_string += wxString(_("XORKey:")) + wxT("\t") + XORKey + wxT("\n");

	m_InfoPanelText->SetLabel(info_string);

#ifdef _DEBUG_
	std::cout << flnm.GetPath().ToAscii() << ' ';
	if (S_ISREG(sbufptr->st_mode))
		printf("regular file");
	else if (S_ISDIR(sbufptr->st_mode))
		printf("directory");
	else if (S_ISCHR(sbufptr->st_mode))
		printf("character device");
	else if (S_ISBLK(sbufptr->st_mode)) {
		printf("block device");
	}
	else if (S_ISFIFO(sbufptr->st_mode))
		printf("FIFO");
#ifndef __WXMSW__
	else if (S_ISLNK(sbufptr->st_mode))
		printf("symbolic link");
	else if (S_ISSOCK(sbufptr->st_mode))
		printf("socket");
#endif
	printf("\n");
#endif
	//		S_IFMT 	0170000 	bitmask for the file type bitfields
	//		S_IFSOCK 	0140000 	socket
	//		S_IFLNK 	0120000 	symbolic link
	//		S_IFREG 	0100000 	regular file
	//		S_IFBLK 	0060000 	block device
	//		S_IFDIR 	0040000 	directory
	//		S_IFCHR 	0020000 	character device
	//		S_IFIFO 	0010000 	FIFO
	//		S_ISUID 	0004000 	set UID bit
	//		S_ISGID 	0002000 	set-group-ID bit (see below)
	//		S_ISVTX 	0001000 	sticky bit (see below)
	//		S_IRWXU 	00700 	mask for file owner permissions
	//		S_IRUSR 	00400 	owner has read permission
	//		S_IWUSR 	00200 	owner has write permission
	//		S_IXUSR 	00100 	owner has execute permission
	//		S_IRWXG 	00070 	mask for group permissions
	//		S_IRGRP 	00040 	group has read permission
	//		S_IWGRP 	00020 	group has write permission
	//		S_IXGRP 	00010 	group has execute permission
	//		S_IRWXO 	00007 	mask for permissions for others (not in group)
	//		S_IROTH 	00004 	others have read permission
	//		S_IWOTH 	00002 	others have write permission
	//		S_IXOTH 	00001 	others have execute permission
	delete sbufptr;
	mutexinfo.Unlock();
}

DisassemblerPanelGUI::DisassemblerPanelGUI(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizerTop;
	bSizerTop = new wxBoxSizer(wxHORIZONTAL);

	wxString m_choiceVendorChoices[] = { wxT("INTEL"), wxT("AMD") };
	int m_choiceVendorNChoices = sizeof(m_choiceVendorChoices) / sizeof(wxString);
	
	m_ChoiceVendor = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceVendorNChoices, m_choiceVendorChoices, 0);
	m_ChoiceVendor->SetSelection(0);
	m_ChoiceVendor->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_ChoiceVendor->SetToolTip(wxT("CPU Vendor"));

	bSizerTop->Add(m_ChoiceVendor, 1, wxTOP | wxRIGHT | wxLEFT, 5);

	wxString m_choiceASMTypeChoices[] = { wxT("INTEL"), wxT("AT&T") };
	int m_choiceASMTypeNChoices = sizeof(m_choiceASMTypeChoices) / sizeof(wxString);
	m_ChoiceASMType = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceASMTypeNChoices, m_choiceASMTypeChoices, 0);
	m_ChoiceASMType->SetSelection(0);
	m_ChoiceASMType->SetToolTip(wxT("Assembly Type"));

	bSizerTop->Add(m_ChoiceASMType, 1, wxTOP | wxRIGHT | wxLEFT, 5);

	wxString m_choiceBitModeChoices[] = { wxT("16 Bit"), wxT("32 Bit"), wxT("64 Bit") };
	int m_choiceBitModeNChoices = sizeof(m_choiceBitModeChoices) / sizeof(wxString);
	m_ChoiceBitMode = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceBitModeNChoices, m_choiceBitModeChoices, 0);
	m_ChoiceBitMode->SetSelection(1);
	m_ChoiceBitMode->SetToolTip(wxT("Disassembly Bit Mode"));

	bSizerTop->Add(m_ChoiceBitMode, 1, wxTOP | wxRIGHT | wxLEFT, 5);


	mainSizer->Add(bSizerTop, 0, wxEXPAND, 5);

	m_DasmCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP | wxTE_MULTILINE | wxTE_READONLY);
	mainSizer->Add(m_DasmCtrl, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 5);


	this->SetSizer(mainSizer);
	this->Layout();

	// Connect Events
	m_ChoiceVendor->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);
	m_ChoiceASMType->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);
	m_ChoiceBitMode->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);
}

DisassemblerPanelGUI::~DisassemblerPanelGUI()
{
	// Disconnect Events
	m_ChoiceVendor->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);
	m_ChoiceASMType->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);
	m_ChoiceBitMode->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(DisassemblerPanelGUI::OnUpdate), NULL, this);

}

void DisassemblerPanel::Set(wxMemoryBuffer buff)
{
	m_Buffer = buff;
	wxCommandEvent event;
	OnUpdate(event);
}

void DisassemblerPanel::Clear(void) 
{
	m_DasmCtrl->Clear();
}

void DisassemblerPanel::OnUpdate(wxCommandEvent& event)
{
	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, reinterpret_cast<uint8_t*>(m_Buffer.GetData()), m_Buffer.GetDataLen());
	ud_set_vendor(&ud_obj, (m_ChoiceVendor->GetSelection()) ? UD_VENDOR_AMD : UD_VENDOR_INTEL);
	ud_set_mode(&ud_obj, (m_ChoiceBitMode->GetSelection() + 1) * 16);
	ud_set_syntax(&ud_obj, (m_ChoiceASMType->GetSelection() ? UD_SYN_ATT : UD_SYN_INTEL));
	wxString mydasm;
	while (ud_disassemble(&ud_obj))
		mydasm << wxString::FromAscii(ud_insn_asm(&ud_obj)) << wxT("\n");
	m_DasmCtrl->ChangeValue(mydasm);
}

InterpreterGui::InterpreterGui(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxPanel(parent, id, pos, size, style, name)
{
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* optionSizer;
	optionSizer = new wxBoxSizer(wxHORIZONTAL);

	m_check_unsigned = new wxCheckBox(this, ID_CHK_UNSIGNED, wxT("Unsigned"), wxDefaultPosition, wxDefaultSize, 0);
	m_check_unsigned->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));

	optionSizer->Add(m_check_unsigned, 0, wxALL, 5);

	m_check_bigendian = new wxCheckBox(this, ID_CHK_BIGENDIAN, wxT("Big Endian"), wxDefaultPosition, wxDefaultSize, 0);
	m_check_bigendian->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	optionSizer->Add(m_check_bigendian, 0, wxALL, 5);


	mainSizer->Add(optionSizer, 0, wxEXPAND, 4);

	wxFlexGridSizer* numSizer;
	numSizer = new wxFlexGridSizer(0, 2, 0, 0);
	numSizer->AddGrowableCol(1);
	numSizer->SetFlexibleDirection(wxHORIZONTAL);
	numSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);

	m_static_bin = new wxStaticText(this, ID_DEFAULT, wxT("Binary"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_bin->Wrap(-1);
	m_static_bin->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_bin, 0, wxALIGN_CENTER, 2);

	wxFlexGridSizer* fgBinSizer;
	fgBinSizer = new wxFlexGridSizer(1, 2, 0, 0);
	fgBinSizer->AddGrowableCol(0);
	fgBinSizer->SetFlexibleDirection(wxBOTH);
	fgBinSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_textctrl_binary = new wxTextCtrl(this, ID_DEFAULT, wxT("00000000"), wxDefaultPosition, wxSize(-1, -1), 0);
#ifdef __WXGTK__
	if (!m_textctrl_binary->HasFlag(wxTE_MULTILINE))
	{
		m_textctrl_binary->SetMaxLength(8);
	}
#else
	m_textctrl_binary->SetMaxLength(8);
#endif
	m_textctrl_binary->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_textctrl_binary->SetToolTip(wxT("Press enter to make changes!"));

	fgBinSizer->Add(m_textctrl_binary, 0, wxEXPAND, 1);

	m_check_edit = new wxCheckBox(this, wxID_ANY, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
	m_check_edit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_check_edit->SetToolTip(wxT("Allow editing by Data Interpreter Panel"));

	fgBinSizer->Add(m_check_edit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 1);


	numSizer->Add(fgBinSizer, 1, wxEXPAND, 5);

	m_static_ascii = new wxStaticText(this, ID_DEFAULT, wxT("ASCII"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_ascii->Wrap(-1);
	m_static_ascii->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_ascii, 0, wxALIGN_CENTER, 5);

	m_textctrl_ascii = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_textctrl_ascii->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_ascii, 0, wxEXPAND, 1);

	m_static_8bit = new wxStaticText(this, ID_DEFAULT, wxT("8 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_8bit->Wrap(-1);
	m_static_8bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_8bit, 0, wxALIGN_CENTER, 0);

	m_textctrl_8bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_textctrl_8bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_8bit, 0, wxEXPAND, 1);

	m_static_16bit = new wxStaticText(this, ID_DEFAULT, wxT("16 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_16bit->Wrap(-1);
	m_static_16bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_16bit, 0, wxALIGN_CENTER, 0);

	m_textctrl_16bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_textctrl_16bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_16bit, 0, wxEXPAND, 1);

	m_static_32bit = new wxStaticText(this, ID_DEFAULT, wxT("32 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_32bit->Wrap(-1);
	m_static_32bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_32bit, 0, wxALIGN_CENTER, 2);

	m_textctrl_32bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_32bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_32bit, 0, wxEXPAND, 1);

	m_static_64bit = new wxStaticText(this, ID_DEFAULT, wxT("64 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_64bit->Wrap(-1);
	m_static_64bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_64bit, 0, wxALIGN_CENTER, 2);

	m_textctrl_64bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_64bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_64bit, 0, wxEXPAND, 1);

	m_static_float = new wxStaticText(this, ID_DEFAULT, wxT("Float"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_float->Wrap(-1);
	m_static_float->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_float, 0, wxALIGN_CENTER, 2);

	m_textctrl_float = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_float->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_float, 0, wxEXPAND, 1);

	m_static_double = new wxStaticText(this, ID_DEFAULT, wxT("Double"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_static_double->Wrap(-1);
	m_static_double->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_static_double, 0, wxALIGN_CENTER, 3);

	m_textctrl_double = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_double->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_textctrl_double, 0, wxEXPAND, 1);


	mainSizer->Add(numSizer, 0, wxEXPAND, 5);

	m_collapsiblePane_TimeMachine = new wxCollapsiblePane(this, wxID_ANY, wxT("Time Machine"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	m_collapsiblePane_TimeMachine->Collapse(false);

	m_collapsiblePane_TimeMachine->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_collapsiblePane_TimeMachine->Hide();

	wxBoxSizer* bSizerTimeMachine;
	bSizerTimeMachine = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizerUTC;
	fgSizerUTC = new wxFlexGridSizer(0, 5, 0, 0);
	fgSizerUTC->SetFlexibleDirection(wxBOTH);
	fgSizerUTC->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_checkBoxLocal = new wxCheckBox(m_collapsiblePane_TimeMachine->GetPane(), wxID_ANY, wxT("Use local time"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	fgSizerUTC->Add(m_checkBoxLocal, 0, wxALL, 5);

	m_panel_time = new wxPanel(m_collapsiblePane_TimeMachine->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	fgSizerUTC->Add(m_panel_time, 1, wxEXPAND | wxALL, 5);

	m_static_timeUTC = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("UTC"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeUTC->Wrap(-1);
	m_static_timeUTC->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgSizerUTC->Add(m_static_timeUTC, 0, wxALIGN_CENTER | wxALL, 5);

	m_spinCtrl_timeUTC = new wxSpinCtrl(m_collapsiblePane_TimeMachine->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxALIGN_RIGHT | wxSP_ARROW_KEYS, -12, 12, 0);
	fgSizerUTC->Add(m_spinCtrl_timeUTC, 0, 0, 5);


	bSizerTimeMachine->Add(fgSizerUTC, 0, wxEXPAND, 5);

	wxFlexGridSizer* fgtimeSizer;
	fgtimeSizer = new wxFlexGridSizer(0, 2, 0, 0);
	fgtimeSizer->AddGrowableCol(1);
	fgtimeSizer->SetFlexibleDirection(wxHORIZONTAL);
	fgtimeSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_static_timeUnix = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("Unix32"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeUnix->Wrap(-1);
	m_static_timeUnix->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeUnix, 1, wxALIGN_CENTER, 5);

	m_textctrl_timeUnix = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeUnix->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeUnix, 0, wxEXPAND, 5);

	m_static_timeUnix64 = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("Unix64"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeUnix64->Wrap(-1);
	m_static_timeUnix64->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeUnix64, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeUnix64 = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeUnix64->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeUnix64, 0, wxEXPAND, 5);

	m_static_timeFAT = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("FAT"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeFAT->Wrap(-1);
	m_static_timeFAT->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeFAT, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeFAT = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeFAT->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeFAT, 0, wxEXPAND, 5);

	m_static_timeNTFS = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("NTFS"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeNTFS->Wrap(-1);
	m_static_timeNTFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeNTFS, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeNTFS = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeNTFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeNTFS, 0, wxEXPAND, 5);

	m_static_timeHFSp = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("HFS+"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeHFSp->Wrap(-1);
	m_static_timeHFSp->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeHFSp, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeHFSp = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeHFSp->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeHFSp, 0, wxEXPAND, 5);

	m_static_timeAPFS = new wxStaticText(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxT("APFS"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeAPFS->Wrap(-1);
	m_static_timeAPFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_static_timeAPFS, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeAPFS = new wxTextCtrl(m_collapsiblePane_TimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeAPFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_textctrl_timeAPFS, 0, wxEXPAND, 5);


	bSizerTimeMachine->Add(fgtimeSizer, 0, wxEXPAND, 5);

	m_collapsiblePaneExFAT = new wxCollapsiblePane(m_collapsiblePane_TimeMachine->GetPane(), wxID_ANY, wxT("exFAT Time & Date"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	m_collapsiblePaneExFAT->Collapse(false);

	m_collapsiblePaneExFAT->Hide();

	wxFlexGridSizer* fgtimeSizer1;
	fgtimeSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	fgtimeSizer1->AddGrowableCol(1);
	fgtimeSizer1->SetFlexibleDirection(wxHORIZONTAL);
	fgtimeSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_static_timeExFAT_Creation = new wxStaticText(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Creation"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeExFAT_Creation->Wrap(-1);
	m_static_timeExFAT_Creation->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_static_timeExFAT_Creation, 1, wxALIGN_CENTER, 5);

	m_textctrl_timeExFAT_Creation = new wxTextCtrl(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeExFAT_Creation->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_textctrl_timeExFAT_Creation, 0, wxEXPAND, 5);

	m_static_timeExFAT_Modification = new wxStaticText(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Modification"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeExFAT_Modification->Wrap(-1);
	m_static_timeExFAT_Modification->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_static_timeExFAT_Modification, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeExFAT_Modification = new wxTextCtrl(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeExFAT_Modification->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_textctrl_timeExFAT_Modification, 0, wxEXPAND, 5);

	m_static_timeExFAT_Access = new wxStaticText(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Access"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_static_timeExFAT_Access->Wrap(-1);
	m_static_timeExFAT_Access->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_static_timeExFAT_Access, 0, wxALIGN_CENTER, 5);

	m_textctrl_timeExFAT_Access = new wxTextCtrl(m_collapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textctrl_timeExFAT_Access->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_textctrl_timeExFAT_Access, 0, wxEXPAND, 5);


	m_collapsiblePaneExFAT->GetPane()->SetSizer(fgtimeSizer1);
	m_collapsiblePaneExFAT->GetPane()->Layout();
	fgtimeSizer1->Fit(m_collapsiblePaneExFAT->GetPane());
	bSizerTimeMachine->Add(m_collapsiblePaneExFAT, 1, wxEXPAND | wxALL, 5);


	m_collapsiblePane_TimeMachine->GetPane()->SetSizer(bSizerTimeMachine);
	m_collapsiblePane_TimeMachine->GetPane()->Layout();
	bSizerTimeMachine->Fit(m_collapsiblePane_TimeMachine->GetPane());
	mainSizer->Add(m_collapsiblePane_TimeMachine, 1, wxEXPAND, 5);


	this->SetSizer(mainSizer);
	this->Layout();
	mainSizer->Fit(this);

	// Connect Events
	m_check_unsigned->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_check_bigendian->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_textctrl_binary->Connect(wxEVT_CHAR, wxKeyEventHandler(InterpreterGui::OnTextEdit), NULL, this);
	m_textctrl_binary->Connect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Connect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_check_edit->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnCheckEdit), NULL, this);
	m_checkBoxLocal->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_spinCtrl_timeUTC->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(InterpreterGui::OnSpin), NULL, this);
}

InterpreterGui::~InterpreterGui()
{
	// Disconnect Events
	m_check_unsigned->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_check_bigendian->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_CHAR, wxKeyEventHandler(InterpreterGui::OnTextEdit), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_MIDDLE_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_textctrl_binary->Disconnect(wxEVT_RIGHT_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_check_edit->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnCheckEdit), NULL, this);
	m_checkBoxLocal->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_spinCtrl_timeUTC->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(InterpreterGui::OnSpin), NULL, this);

}
