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
#include <wx/collpane.h>
#include <wx/spinctrl.h>
#include <wx/filehistory.h>
#include <types.h>
#include <extern.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "UDKChief.h"
#include "UDKControl.h"

#include "UDKHexEditor.h"

/** @file
 * @brief UDKApplication code
 *
 * Contains the main() and associated code for UDKApplication.
 */

UDKHalo* UDKApplication::m_Frame = nullptr;

wxIMPLEMENT_APP_CONSOLE(UDKApplication);

// Global variable(s)
int FakeBlockSize = 0;

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

	// Instantiate a notebook
	m_IDANotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT | wxAUI_NB_WINDOWLIST_BUTTON);

	// Set panes and panels
	PrepareAUI();

	// Memorize recently opened files
	m_MenuFileOpenRecent = new wxMenu();
	wxMenuItem* menuFileOpenRecentItem = new wxMenuItem(menuFile, wxID_ANY, wxT("Open &Recent"), wxEmptyString, wxITEM_NORMAL, m_MenuFileOpenRecent);
	wxMenuItem* menuFileOpenRecentDummy;
	menuFileOpenRecentDummy = new wxMenuItem(m_MenuFileOpenRecent, wxID_ANY, wxString( wxT("No Recent File") ) , wxEmptyString, wxITEM_NORMAL );
	m_MenuFileOpenRecent->Append(menuFileOpenRecentDummy);
	menuFileOpenRecentDummy->Enable( false );

	menuBar->Append(m_MenuFileOpenRecent, "&Recent");

	// Setup history
	m_FileHistory = new wxFileHistory();
	m_FileHistory->UseMenu(m_MenuFileOpenRecent);
	m_MenuFileOpenRecent->Remove(*m_MenuFileOpenRecent->GetMenuItems().begin() ); //Removes "no recent file" message
	m_FileHistory->Load(*MyConfigBase::Get());

	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);

	m_StatusBar = CreateStatusBar();
	SetStatusText("Welcome to Unreal DeKompiler!");

	Bind(wxEVT_MENU, &UDKHalo::OnHello, this, ID_Hello);
	Bind(wxEVT_MENU, &UDKHalo::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &UDKHalo::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &UDKHalo::OnOpenFile, this, wxID_OPEN);
}

UDKHexEditor* UDKHalo::GetActiveHexEditor(void)
{
	// TODO (death#1#): BUG : MyNotebook = warning RTTI symbol not found for class wxAuiFloatingFrame
	int x = m_IDANotebook->GetSelection();
	return x == wxNOT_FOUND ? NULL : static_cast<UDKHexEditor*>(m_IDANotebook->GetPage(x));
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
		else
		{
			OpenFile(filename, true);
		}
	}
}

UDKHexEditor* UDKHalo::OpenFile(wxFileName filename, bool openAtRight)
{
	UDKHexEditor *x = new UDKHexEditor(m_IDANotebook, -1, m_StatusBar, nullptr, m_FileInfoPanel, nullptr, m_DisassemblerPanel, nullptr);
	x->Hide();//Hiding hex editor for avoiding visual artifacts on loading file...

	if(!filename.GetName().StartsWith(wxT("-buf")) && !filename.GetName().StartsWith(wxT("-pid")))
	{
		if(filename.IsRelative()) //Make Relative path to Absolute
		{
			filename.Normalize();
		}
	}

	if(x->FileOpen(filename))
	{
		m_IDANotebook->AddPage(x, x->GetFileName().GetFullName(), true);
		x->Show();
		if(openAtRight)
		{
			m_IDANotebook->Split(m_IDANotebook->GetSelection(), wxRIGHT);
		}

		bool autoShowTagsSwitch;
		MyConfigBase::Get()->Read( _T("AutoShowTagPanel"), &autoShowTagsSwitch, true );

		//Detect from file name if we are opening a RAM Process:
		if((x->m_MainTagArray.Count() > 0 && autoShowTagsSwitch) || filename.GetFullPath().Lower().StartsWith( wxT("-pid=")))
		{
			//m_PaneManager->GetPane(MyTagPanel).Show( true );
			m_PaneManager->Update();
		}

		int found = -1;

		//For loop updates Open Recent Menu properly.
		for( unsigned i=0; i < m_FileHistory->GetCount() ; i++)
		{
			if(m_FileHistory->GetHistoryFile( i ) == filename.GetFullPath())
			{
				found = i;
			}
		}

		if( found != -1 )
		{
			m_FileHistory->RemoveFileFromHistory( found );
		}

		m_FileHistory->AddFileToHistory( filename.GetFullPath() );
		m_FileHistory->Save(*(MyConfigBase::Get()));

		MyConfigBase::Get()->Flush();

		if(wxFileExists(filename.GetFullPath().Append(wxT(".md5"))))
		{
			if(wxYES == wxMessageBox(_("MD5 File detected. Do you request MD5 verification?"), _("Checksum File Detected"), wxYES_NO|wxNO_DEFAULT, this))
			{
				x->HashVerify(filename.GetFullPath().Append(wxT(".md5")));
			}
		}
		if(wxFileExists(filename.GetFullPath().Append(wxT(".sha1"))))
		{
			if(wxYES==wxMessageBox(_("SHA1 File detected. Do you request SHA1 verification?"), _("Checksum File Detected"), wxYES_NO|wxNO_DEFAULT, this ))
			{
				x->HashVerify(filename.GetFullPath().Append(wxT(".sha1")));
			}
		}
		if(wxFileExists( filename.GetFullPath().Append(wxT(".sha256"))))
		{
			if(wxYES==wxMessageBox(_("SHA256 File detected. Do you request SHA256 verification?"), _("Checksum File Detected"), wxYES_NO|wxNO_DEFAULT, this ))
			{
				x->HashVerify(filename.GetFullPath().Append(wxT(".sha256")));
			}
		}

#if _FSWATCHER_
		if(x->GetFileType()==FAL::FAL_File) {
			//file_watcher->Add( filename );
			file_watcher->Add( filename, wxFSW_EVENT_MODIFY );
			file_watcher->Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(HexEditor::OnFileModify), NULL, x);
			}
		else {
			std::cout << "File_watcher event is null! File Watcher is not working!" << std::endl;
			}
#endif // _FSWATCHER_
		//ActionEnabler();
		return x;
	}
	else
	{
		x->Destroy();
		return NULL;
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

	m_CheckUnsigned = new wxCheckBox(this, ID_CHK_UNSIGNED, wxT("Unsigned"), wxDefaultPosition, wxDefaultSize, 0);
	m_CheckUnsigned->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));

	optionSizer->Add(m_CheckUnsigned, 0, wxALL, 5);

	m_CheckBigEndian = new wxCheckBox(this, ID_CHK_BIGENDIAN, wxT("Big Endian"), wxDefaultPosition, wxDefaultSize, 0);
	m_CheckBigEndian->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	optionSizer->Add(m_CheckBigEndian, 0, wxALL, 5);


	mainSizer->Add(optionSizer, 0, wxEXPAND, 4);

	wxFlexGridSizer* numSizer;
	numSizer = new wxFlexGridSizer(0, 2, 0, 0);
	numSizer->AddGrowableCol(1);
	numSizer->SetFlexibleDirection(wxHORIZONTAL);
	numSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);

	m_StaticBin = new wxStaticText(this, ID_DEFAULT, wxT("Binary"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_StaticBin->Wrap(-1);
	m_StaticBin->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_StaticBin, 0, wxALIGN_CENTER, 2);

	wxFlexGridSizer* fgBinSizer;
	fgBinSizer = new wxFlexGridSizer(1, 2, 0, 0);
	fgBinSizer->AddGrowableCol(0);
	fgBinSizer->SetFlexibleDirection(wxBOTH);
	fgBinSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_TextControlBinary = new wxTextCtrl(this, ID_DEFAULT, wxT("00000000"), wxDefaultPosition, wxSize(-1, -1), 0);
#ifdef __WXGTK__
	if (!m_TextControlBinary->HasFlag(wxTE_MULTILINE))
	{
		m_TextControlBinary->SetMaxLength(8);
	}
#else
	m_TextControlBinary->SetMaxLength(8);
#endif
	m_TextControlBinary->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_TextControlBinary->SetToolTip(wxT("Press enter to make changes!"));

	fgBinSizer->Add(m_TextControlBinary, 0, wxEXPAND, 1);

	m_CheckEdit = new wxCheckBox(this, wxID_ANY, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
	m_CheckEdit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_CheckEdit->SetToolTip(wxT("Allow editing by Data Interpreter Panel"));

	fgBinSizer->Add(m_CheckEdit, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 1);


	numSizer->Add(fgBinSizer, 1, wxEXPAND, 5);

	m_StaticAscii = new wxStaticText(this, ID_DEFAULT, wxT("ASCII"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_StaticAscii->Wrap(-1);
	m_StaticAscii->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_StaticAscii, 0, wxALIGN_CENTER, 5);

	m_TextControlAscii = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_TextControlAscii->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControlAscii, 0, wxEXPAND, 1);

	m_Static8bit = new wxStaticText(this, ID_DEFAULT, wxT("8 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Static8bit->Wrap(-1);
	m_Static8bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Static8bit, 0, wxALIGN_CENTER, 0);

	m_TextControl8bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_TextControl8bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControl8bit, 0, wxEXPAND, 1);

	m_Static16bit = new wxStaticText(this, ID_DEFAULT, wxT("16 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Static16bit->Wrap(-1);
	m_Static16bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Static16bit, 0, wxALIGN_CENTER, 0);

	m_TextControl16bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_READONLY);
	m_TextControl16bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControl16bit, 0, wxEXPAND, 1);

	m_Static32bit = new wxStaticText(this, ID_DEFAULT, wxT("32 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Static32bit->Wrap(-1);
	m_Static32bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Static32bit, 0, wxALIGN_CENTER, 2);

	m_TextControl32bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControl32bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControl32bit, 0, wxEXPAND, 1);

	m_Static64bit = new wxStaticText(this, ID_DEFAULT, wxT("64 bit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Static64bit->Wrap(-1);
	m_Static64bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Static64bit, 0, wxALIGN_CENTER, 2);

	m_TextControl64bit = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControl64bit->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControl64bit, 0, wxEXPAND, 1);

	m_Staticfloat = new wxStaticText(this, ID_DEFAULT, wxT("Float"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Staticfloat->Wrap(-1);
	m_Staticfloat->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Staticfloat, 0, wxALIGN_CENTER, 2);

	m_TextControlfloat = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlfloat->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControlfloat, 0, wxEXPAND, 1);

	m_Staticdouble = new wxStaticText(this, ID_DEFAULT, wxT("Double"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_Staticdouble->Wrap(-1);
	m_Staticdouble->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_Staticdouble, 0, wxALIGN_CENTER, 3);

	m_TextControldouble = new wxTextCtrl(this, ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControldouble->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	numSizer->Add(m_TextControldouble, 0, wxEXPAND, 1);


	mainSizer->Add(numSizer, 0, wxEXPAND, 5);

	m_CollapsiblePaneTimeMachine = new wxCollapsiblePane(this, wxID_ANY, wxT("Time Machine"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	m_CollapsiblePaneTimeMachine->Collapse(false);

	m_CollapsiblePaneTimeMachine->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	m_CollapsiblePaneTimeMachine->Hide();

	wxBoxSizer* bSizerTimeMachine;
	bSizerTimeMachine = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizerUTC;
	fgSizerUTC = new wxFlexGridSizer(0, 5, 0, 0);
	fgSizerUTC->SetFlexibleDirection(wxBOTH);
	fgSizerUTC->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_CheckBoxLocal = new wxCheckBox(m_CollapsiblePaneTimeMachine->GetPane(), wxID_ANY, wxT("Use local time"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	fgSizerUTC->Add(m_CheckBoxLocal, 0, wxALL, 5);

	m_PanelTime = new wxPanel(m_CollapsiblePaneTimeMachine->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	fgSizerUTC->Add(m_PanelTime, 1, wxEXPAND | wxALL, 5);

	m_StaticTimeUTC = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("UTC"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeUTC->Wrap(-1);
	m_StaticTimeUTC->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgSizerUTC->Add(m_StaticTimeUTC, 0, wxALIGN_CENTER | wxALL, 5);

	m_SpinControlTimeUTC = new wxSpinCtrl(m_CollapsiblePaneTimeMachine->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxALIGN_RIGHT | wxSP_ARROW_KEYS, -12, 12, 0);
	fgSizerUTC->Add(m_SpinControlTimeUTC, 0, 0, 5);


	bSizerTimeMachine->Add(fgSizerUTC, 0, wxEXPAND, 5);

	wxFlexGridSizer* fgtimeSizer;
	fgtimeSizer = new wxFlexGridSizer(0, 2, 0, 0);
	fgtimeSizer->AddGrowableCol(1);
	fgtimeSizer->SetFlexibleDirection(wxHORIZONTAL);
	fgtimeSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_StaticTimeUnix = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("Unix32"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeUnix->Wrap(-1);
	m_StaticTimeUnix->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeUnix, 1, wxALIGN_CENTER, 5);

	m_TextControlTimeUnix = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeUnix->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeUnix, 0, wxEXPAND, 5);

	m_StaticTimeUnix64 = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("Unix64"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeUnix64->Wrap(-1);
	m_StaticTimeUnix64->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeUnix64, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeUnix64 = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeUnix64->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeUnix64, 0, wxEXPAND, 5);

	m_StaticTimeFAT = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("FAT"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeFAT->Wrap(-1);
	m_StaticTimeFAT->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeFAT, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeFAT = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeFAT->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeFAT, 0, wxEXPAND, 5);

	m_StaticTimeNTFS = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("NTFS"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeNTFS->Wrap(-1);
	m_StaticTimeNTFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeNTFS, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeNTFS = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeNTFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeNTFS, 0, wxEXPAND, 5);

	m_StaticTimeHFSp = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("HFS+"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeHFSp->Wrap(-1);
	m_StaticTimeHFSp->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeHFSp, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeHFSp = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeHFSp->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeHFSp, 0, wxEXPAND, 5);

	m_StaticTimeAPFS = new wxStaticText(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxT("APFS"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeAPFS->Wrap(-1);
	m_StaticTimeAPFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_StaticTimeAPFS, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeAPFS = new wxTextCtrl(m_CollapsiblePaneTimeMachine->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeAPFS->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer->Add(m_TextControlTimeAPFS, 0, wxEXPAND, 5);


	bSizerTimeMachine->Add(fgtimeSizer, 0, wxEXPAND, 5);

	m_CollapsiblePaneExFAT = new wxCollapsiblePane(m_CollapsiblePaneTimeMachine->GetPane(), wxID_ANY, wxT("exFAT Time & Date"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	m_CollapsiblePaneExFAT->Collapse(false);

	m_CollapsiblePaneExFAT->Hide();

	wxFlexGridSizer* fgtimeSizer1;
	fgtimeSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	fgtimeSizer1->AddGrowableCol(1);
	fgtimeSizer1->SetFlexibleDirection(wxHORIZONTAL);
	fgtimeSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_StaticTimeExFATCreation = new wxStaticText(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Creation"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeExFATCreation->Wrap(-1);
	m_StaticTimeExFATCreation->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_StaticTimeExFATCreation, 1, wxALIGN_CENTER, 5);

	m_TextControlTimeExFATCreation = new wxTextCtrl(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeExFATCreation->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_TextControlTimeExFATCreation, 0, wxEXPAND, 5);

	m_StaticTimeExFATModification = new wxStaticText(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Modification"), wxPoint(-1, -1), wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeExFATModification->Wrap(-1);
	m_StaticTimeExFATModification->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_StaticTimeExFATModification, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeExFATModification = new wxTextCtrl(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeExFATModification->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_TextControlTimeExFATModification, 0, wxEXPAND, 5);

	m_StaticTimeExFATAccess = new wxStaticText(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxT("Access"), wxDefaultPosition, wxSize(-1, -1), wxALIGN_LEFT);
	m_StaticTimeExFATAccess->Wrap(-1);
	m_StaticTimeExFATAccess->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_StaticTimeExFATAccess, 0, wxALIGN_CENTER, 5);

	m_TextControlTimeExFATAccess = new wxTextCtrl(m_CollapsiblePaneExFAT->GetPane(), ID_DEFAULT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_TextControlTimeExFATAccess->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	fgtimeSizer1->Add(m_TextControlTimeExFATAccess, 0, wxEXPAND, 5);


	m_CollapsiblePaneExFAT->GetPane()->SetSizer(fgtimeSizer1);
	m_CollapsiblePaneExFAT->GetPane()->Layout();
	fgtimeSizer1->Fit(m_CollapsiblePaneExFAT->GetPane());
	bSizerTimeMachine->Add(m_CollapsiblePaneExFAT, 1, wxEXPAND | wxALL, 5);


	m_CollapsiblePaneTimeMachine->GetPane()->SetSizer(bSizerTimeMachine);
	m_CollapsiblePaneTimeMachine->GetPane()->Layout();
	bSizerTimeMachine->Fit(m_CollapsiblePaneTimeMachine->GetPane());
	mainSizer->Add(m_CollapsiblePaneTimeMachine, 1, wxEXPAND, 5);


	this->SetSizer(mainSizer);
	this->Layout();
	mainSizer->Fit(this);

	// Connect Events
	m_CheckUnsigned->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_CheckBigEndian->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_TextControlBinary->Connect(wxEVT_CHAR, wxKeyEventHandler(InterpreterGui::OnTextEdit), NULL, this);
	m_TextControlBinary->Connect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Connect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_CheckEdit->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnCheckEdit), NULL, this);
	m_CheckBoxLocal->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_SpinControlTimeUTC->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(InterpreterGui::OnSpin), NULL, this);
}

InterpreterGui::~InterpreterGui()
{
	// Disconnect Events
	m_CheckUnsigned->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_CheckBigEndian->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_CHAR, wxKeyEventHandler(InterpreterGui::OnTextEdit), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_MIDDLE_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_TextControlBinary->Disconnect(wxEVT_RIGHT_UP, wxMouseEventHandler(InterpreterGui::OnTextMouse), NULL, this);
	m_CheckEdit->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnCheckEdit), NULL, this);
	m_CheckBoxLocal->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(InterpreterGui::OnUpdate), NULL, this);
	m_SpinControlTimeUTC->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(InterpreterGui::OnSpin), NULL, this);
}

void DataInterpreter::Set(wxMemoryBuffer buffer)
{
	// TODO (death#1#): Add exception if size smaller than expected
	static wxMutex mutexset;
#ifdef _DEBUG_MUTEX_
	std::cout << "DataInterpeter Set() Mutex Locked" << std::endl;
#endif
	mutexset.Lock();

	size_t size = buffer.GetDataLen();
	if (size == 0)
	{
		wxBell();
		Clear();
		mutexset.Unlock();
#ifdef _DEBUG_MUTEX_
		std::cout << "DataInterpeter Set() Mutex UnLocked" << std::endl;
#endif
		return;
	}
	if (unidata.raw != NULL)
		delete[] unidata.raw;
	if (unidata.mraw != NULL)
		delete[] unidata.mraw;
	unidata.raw = new char[size];
	unidata.mraw = new char[size];
	memcpy(unidata.raw, buffer.GetData(), size);
	memcpy(unidata.mraw, buffer.GetData(), size);
	unidata.size = size;
	for (int i = 0; i < unidata.size; i++)	// make mirror image of mydata
		unidata.mraw[i] = unidata.raw[unidata.size - i - 1];

	unidata.little.bit8 = reinterpret_cast<int8_t*>(unidata.raw);
	unidata.little.ubit8 = reinterpret_cast<uint8_t*>(unidata.raw);
	unidata.little.bit16 = reinterpret_cast<int16_t*>(unidata.raw);
	unidata.little.ubit16 = reinterpret_cast<uint16_t*>(unidata.raw);
	unidata.little.bit32 = reinterpret_cast<int32_t*>(unidata.raw);
	unidata.little.ubit32 = reinterpret_cast<uint32_t*>(unidata.raw);
	unidata.little.bit64 = reinterpret_cast<int64_t*>(unidata.raw);
	unidata.little.ubit64 = reinterpret_cast<uint64_t*>(unidata.raw);
	unidata.little.bitfloat = reinterpret_cast<float*>(unidata.raw);
	unidata.little.bitdouble = reinterpret_cast<double*>(unidata.raw);
	//unidata.little.bitf128 = reinterpret_cast< _Float128* >(unidata.raw);
	unidata.little.raw = reinterpret_cast<char*>(unidata.raw);

	unidata.big.bit8 = reinterpret_cast<int8_t*>(unidata.mraw + (size - 1));
	unidata.big.ubit8 = reinterpret_cast<uint8_t*>(unidata.mraw + (size - 1));
	unidata.big.bit16 = reinterpret_cast<int16_t*>(unidata.mraw + (size - 2));
	unidata.big.ubit16 = reinterpret_cast<uint16_t*>(unidata.mraw + (size - 2));
	unidata.big.bit32 = reinterpret_cast<int32_t*>(unidata.mraw + (size - 4));
	unidata.big.ubit32 = reinterpret_cast<uint32_t*>(unidata.mraw + (size - 4));
	unidata.big.bit64 = reinterpret_cast<int64_t*>(unidata.mraw + (size - 8));
	unidata.big.ubit64 = reinterpret_cast<uint64_t*>(unidata.mraw + (size - 8));
	unidata.big.bitfloat = reinterpret_cast<float*>(unidata.mraw + (size - 4));
	unidata.big.bitdouble = reinterpret_cast<double*>(unidata.mraw + (size - 8));
	//unidata.big.bitf128 = reinterpret_cast< _Float128* >(unidata.mraw+(size - 16));
	unidata.big.raw = reinterpret_cast<char*>(unidata.raw);

	wxCommandEvent event;
	OnUpdate(event);

	mutexset.Unlock();
#ifdef _DEBUG_MUTEX_
	std::cout << "DataInterpeter Set() Mutex UnLocked" << std::endl;
#endif
}

void DataInterpreter::Clear(void)
{
	m_TextControlBinary->Clear();
	m_TextControl8bit->Clear();
	m_TextControl16bit->Clear();
	m_TextControl32bit->Clear();
	m_TextControl64bit->Clear();
	m_TextControlfloat->Clear();
	m_TextControldouble->Clear();
}

void DataInterpreter::OnUpdate(wxCommandEvent& event)
{
	unidata::endian* X = m_CheckBigEndian->GetValue() ? &unidata.big : &unidata.little;
	int number = *X->ubit8;
	wxString bn;

	for (int i = 8; i > 0; i--)
	{
		(((number >> (i - 1)) & 0x01) == 1) ? bn << wxT("1") : bn << wxT("0");
		//		Disabled shaping due edit function.
		//			if( i == 5 )
		//				bn.append(wxT(" "));
	}
	m_TextControlBinary->ChangeValue(bn);
	if (m_CheckUnsigned->GetValue())
	{
		m_TextControlAscii->ChangeValue(AsciiSymbol(*X->ubit8));
		m_TextControl8bit->ChangeValue(wxString::Format(wxT("%u"), *X->ubit8));
		m_TextControl16bit->ChangeValue(wxString::Format(wxT("%u"), *X->ubit16));
		m_TextControl32bit->ChangeValue(wxString::Format(wxT("%u"), *X->ubit32));
		m_TextControl64bit->ChangeValue(wxString::Format("%" wxLongLongFmtSpec "u", *X->ubit64));
	}
	else
	{
		m_TextControlAscii->ChangeValue(AsciiSymbol(*X->ubit8));
		m_TextControl8bit->ChangeValue(wxString::Format(wxT("%i"), *X->bit8));
		m_TextControl16bit->ChangeValue(wxString::Format(wxT("%i"), *X->bit16));
		m_TextControl32bit->ChangeValue(wxString::Format(wxT("%i"), *X->bit32));
		m_TextControl64bit->ChangeValue(wxString::Format("%" wxLongLongFmtSpec "d", *X->bit64));
	}
	m_TextControlfloat->ChangeValue(wxString::Format(wxT("%.14g"), *X->bitfloat));
	m_TextControldouble->ChangeValue(wxString::Format(wxT("%.14g"), *X->bitdouble));

#ifdef HAS_A_TIME_MACHINE
	m_textctrl_timeUnix->ChangeValue(FluxCapacitor(X, UNIX32));
	m_textctrl_timeUnix64->ChangeValue(FluxCapacitor(X, UNIX64));
	m_textctrl_timeNTFS->ChangeValue(FluxCapacitor(X, NTFS));
	m_textctrl_timeAPFS->ChangeValue(FluxCapacitor(X, APFS));
	m_textctrl_timeHFSp->ChangeValue(FluxCapacitor(X, HFSp));
	m_textctrl_timeFAT->ChangeValue(FluxCapacitor(X, FAT));
#ifdef HAS_A_EXFAT_TIME
	m_textctrl_timeExFAT_Creation->ChangeValue(FluxCapacitor(X, exFAT_C));
	m_textctrl_timeExFAT_Modification->ChangeValue(FluxCapacitor(X, exFAT_M));
	m_textctrl_timeExFAT_Access->ChangeValue(FluxCapacitor(X, exFAT_A));
#endif //exfat
#endif // HAS_A_TIME_MACHINE
}

// TODO (death#1#): Enable Local need to disable UTC ...
//Hide UTC +03's ?
//UTC , Local Time Machine state need to remember
//Silence those assertions!
//Disable resize issue with Time Machine
void DataInterpreter::OnSpin(wxSpinEvent& event)
{
	OnUpdate(event);
}

void DataInterpreter::OnTextEdit(wxKeyEvent& event)
{
	if ((event.GetKeyCode() == '0'
		|| event.GetKeyCode() == '1'
		|| event.GetKeyCode() == WXK_INSERT
		//|| event.GetKeyCode() == WXK_DELETE
		|| event.GetKeyCode() == WXK_END
		|| event.GetKeyCode() == WXK_HOME
		|| event.GetKeyCode() == WXK_LEFT
		|| event.GetKeyCode() == WXK_RIGHT
		//|| event.GetKeyCode() == WXK_BACK
		)
		&& m_CheckEdit->IsChecked())
	{

		event.Skip(); //make updates on binary text control

		//if binary data filled properly, update other text controls
		if (m_TextControlBinary->GetLineLength(0) == 8 && (event.GetKeyCode() == '1' || event.GetKeyCode() == '0'))
		{
			long cursorat = m_TextControlBinary->GetInsertionPoint();
			if (event.GetKeyCode() == '1')
				unidata.raw[0] |= (1 << (7 - cursorat));
			else
				unidata.raw[0] &= ~(1 << (7 - cursorat));

			//unsigned long newlongbyte=0;
			//char newbyte = static_cast<char>(newlongbyte & 0xFF);
			wxMemoryBuffer buffer;
			//buffer.AppendByte( newbyte );
			buffer.AppendData(unidata.raw, unidata.size);
			//if(unidata.size > 1)
			//	buffer.AppendData( unidata.raw+1, unidata.size-1 );
			Set(buffer);
			m_TextControlBinary->SetInsertionPoint(cursorat);
		}
	}
	else if (event.GetKeyCode() == WXK_RETURN && m_TextControlBinary->GetLineLength(0) == 8)
	{
		//Validation
		unsigned long newlongbyte = 0;
		m_TextControlBinary->GetValue().ToULong(&newlongbyte, 2);
		char newbyte = static_cast<char>(newlongbyte & 0xFF);

		UDKHexEditor* hx = static_cast<UDKHalo*>(GetParent())->GetActiveHexEditor();

		hx->FileAddDiff(hx->CursorOffset(), &newbyte, 1);						// add write node to file
		hx->Reload();	//Updates hex editor to show difference.

		wxUpdateUIEvent eventx(UNREDO_EVENT);
		GetEventHandler()->ProcessEvent(eventx);
	}
	else
	{
		wxBell();
	}
}

void DataInterpreter::OnTextMouse(wxMouseEvent& event)
{
	if (event.ButtonDown()) //Just allowed left mouse, setted up by wxFormBuilder.
	{
		wxBell();
	}
	else
	{
		event.Skip();
	}
}

wxString DataInterpreter::AsciiSymbol(unsigned char ch)
{
	static wxString AsciiTable[] = { "NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL","BS","HT","LF","VT",
								"FF","CR","SO","SI","DLE","DC1","DC2","DC3","DC4","NAK","SYN","ETB",
								"CAN","EM","SUB","ESC","FS","GS","RS","US","SP" };//32 SP means SPACE
	if (ch <= 32)
	{
		return AsciiTable[ch];
	}
	else if (ch > 32 && ch < 127)
	{
		return wxString((char)ch);
	}
	else if (ch == 127)
	{
		return wxString("DEL");
	}
	return wxEmptyString;
}

void DataInterpreter::OnCheckEdit(wxCommandEvent& event)
{
	if (event.IsChecked())
	{
		m_TextControlBinary->SetFocus();
		//m_textctrl_binary->SetInsertionPoint(0); //I think this is not needd

// TODO (death#1#): Needed to activate INSERT mode when pressed to Edit check
// TODO (death#1#): Instead change bits by mouse click!
		wxKeyEvent emulate_insert(WXK_INSERT);
		OnTextEdit(emulate_insert);

		///Requires wxTE_MULTILINE!

//		wxTextAttr at;
//		m_textctrl_binary->GetStyle( 0, at );
//		at.SetTextColour( *wxGREEN );
//		m_textctrl_binary->SetStyle( 0,8, at );
//		m_textctrl_binary->SetDefaultStyle( at );
//		m_textctrl_binary->SetValue(m_textctrl_binary->GetValue());
	}
	else
	{
		//		wxTextAttr at;
		//		m_textctrl_binary->GetStyle( 0, at );
		//		at.SetBackgroundColour( *wxRED );
		//		m_textctrl_binary->SetStyle( 0,8, at );
		//		m_textctrl_binary->SetDefaultStyle( at );
		//		m_textctrl_binary->SetValue(m_textctrl_binary->GetValue());
	}
}
