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
#include <types.h>
#include <extern.h>

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

	// Set panes and panels
	PrepareAUI();

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

void UDKHalo::PrepareAUI(void)
{
	m_PaneManager = new wxAuiManager(this);

	m_DisassemblerPanel = new DisassemblerPanel(this, -1);
	m_PaneManager->AddPane(m_DisassemblerPanel, wxAuiPaneInfo().
		Name(_("Disassembler Panel")).
		Caption(_("The Disassembler")).
		TopDockable(false).
		BottomDockable(false).
		MinSize(wxSize(70, 100)).
		BestSize(wxSize(140, 100)).
		Show(false).
		Right().Layer(1));
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
	mybuff = buff;
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
	ud_set_input_buffer(&ud_obj, reinterpret_cast<uint8_t*>(mybuff.GetData()), mybuff.GetDataLen());
	ud_set_vendor(&ud_obj, (m_ChoiceVendor->GetSelection()) ? UD_VENDOR_AMD : UD_VENDOR_INTEL);
	ud_set_mode(&ud_obj, (m_ChoiceBitMode->GetSelection() + 1) * 16);
	ud_set_syntax(&ud_obj, (m_ChoiceASMType->GetSelection() ? UD_SYN_ATT : UD_SYN_INTEL));
	wxString mydasm;
	while (ud_disassemble(&ud_obj))
		mydasm << wxString::FromAscii(ud_insn_asm(&ud_obj)) << wxT("\n");
	m_DasmCtrl->ChangeValue(mydasm);
}