/*
 *   ------------------------
 *  |  UDKEditorControlGui.h
 *   ------------------------
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
 *   March, 2023: Bringing in code from wxHexEditor.
 */

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/panel.h>

#define ID_DEFAULT wxID_ANY // Default
#define ID_HEXBOX 1000
#define ID_TEXTBOX 1001

class UDKControl;
class wxHexOffsetControl;
class wxHexTextControl;

class UDKEditorControlGui : public wxPanel
{
protected:
	wxStaticText* m_StaticOffset;
	wxStaticText* m_StaticAddress;
	wxStaticText* m_StaticByteview;
	wxStaticText* m_StaticNull;
	wxHexOffsetControl* m_OffsetControl;
	UDKControl* m_HexControl;
	wxHexTextControl* m_TextControl;

protected:
	// Virtual event handlers, overide them in your derived class
	virtual void OnKeyboardChar(wxKeyEvent& event) { event.Skip(); }
	virtual void OnResize(wxSizeEvent& event) { event.Skip(); }
	virtual void OnOffsetScroll(wxScrollEvent& event) { event.Skip(); }

public:
	wxScrollBar* m_OffsetScrollReal;

	UDKEditorControlGui(wxWindow* parent, wxWindowID id = ID_DEFAULT, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL);
	~UDKEditorControlGui();
};