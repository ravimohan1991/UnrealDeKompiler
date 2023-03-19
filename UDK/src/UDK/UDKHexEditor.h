/*
 *   ---------------------
 *  |  UDKHexEditor.h
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
 *   March, 2023: Bringing in code from wxHexEditor.
 */

#pragma once

#include "UDKControl.h"

class DisassemblerPanel;
class DataInterpreter;
class InfoPanel;
class TagPanel;
class UDKFile;

WX_DECLARE_OBJARRAY(uint64_t, wxArrayUINT64);

#define SELECT_EVENT 50005

/// <summary>
/// 64bit wrapper for wxScrollbar
/// </summary>
class wxHugeScrollBar : public wxEvtHandler
{
	//friend class wxHexEditorCtrl;
public:
	wxHugeScrollBar(wxScrollBar* m_scrollbar_);
	~wxHugeScrollBar();

	void Enable(bool en)
	{
		m_ScrollBar->Enable(en);
	}
	wxSize GetSize(void)
	{
		return m_ScrollBar->GetSize();
	}
	int64_t GetRange(void)
	{
		return m_Range;
	};
	int64_t GetThumbPosition(void)
	{
		return m_Thumb;
	}
	void SetThumbPosition(int64_t setpos);
	void SetScrollbar(int64_t Current_Position, int page_x, int64_t new_range, int pagesize, bool repaint = true);
	void OnOffsetScroll(wxScrollEvent& event);

private:
	uint64_t				m_Range;
	uint64_t				m_Thumb;

	wxScrollBar*			m_ScrollBar;
};

/// <summary>
/// For using event handler. Need better description
/// </summary>
class Select
{
public:
	Select(wxEvtHandler* evth_)
	{
		m_StartOffset = m_EndOffset = 0;
		m_State = false;
		m_EventHandler = evth_;
	}
	uint64_t GetSize(void)
	{
		return abs(static_cast<int64_t>(m_EndOffset - m_StartOffset)) + 1;
	};//for select byte 13 start=13, end=13

	void SetState(bool new_state)
	{
		m_State = new_state;

		std::cout << "Send UpdateUI Event" << std::endl;
		wxUpdateUIEvent event;
		if (new_state)
			event.SetString(wxT("Selected"));
		else
			event.SetString(wxT("NotSelected"));
		event.SetId(SELECT_EVENT);//idFileRO
		m_EventHandler->ProcessEvent(event);
	}

	inline bool GetState() const
	{
		return m_State;
	}
	
	inline uint64_t GetStart(void) const
	{
		return m_StartOffset < m_EndOffset ? m_StartOffset : m_EndOffset;
	}
	
	inline uint64_t GetEnd(void) const
	{
		return m_StartOffset > m_EndOffset ? m_StartOffset : m_EndOffset;
	}

	uint64_t m_StartOffset;	//real selection start position
	uint64_t m_OriginalStartOffset;  //real selection start position backup for HexTextCTRL
	uint64_t m_EndOffset;		//real selection end position, included to select
private:
	bool  m_State;
	wxEvtHandler* m_EventHandler;
};

/// <summary>
/// Make a copy of the file
/// </summary>
class CopyMaker
{
public:
	bool m_Copied;		//copy in action or not
	uint64_t m_Start;		//copy start position
	uint64_t m_Size;		//size of copy
	wxMemoryBuffer m_Buffer; //uses RAM, for small data
	UDKFile* m_Sourcefile;	//uses HDD File and NOT delete after.
	
	CopyMaker()
	{
		m_Copied = false;
		m_Start = m_Size = 0;
		//	tempfile = NULL;
		m_Sourcefile = NULL;
	}
	~CopyMaker()
	{
	}

	bool SetClipboardData(wxString& CopyString)
	{
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->Clear();
			int isok = wxTheClipboard->SetData(new wxTextDataObject(CopyString));
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
			return isok;
		}
		else
		{
			wxMessageBox(wxString(_("Clipboard could not be opened.")) + wxT("\n") + _("Operation cancelled!"), _("Copy To Clipboard Error"), wxOK | wxICON_ERROR);
			return false;
		}
	}

	wxString GetClipboardData(void)
	{
		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported(wxDF_TEXT))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				wxTheClipboard->Close();
				return data.GetText();
			}
			else
			{
				wxBell();
				wxTheClipboard->Close();
				return wxString();
			}
		}
		else
		{
			wxMessageBox(wxString(_("Clipboard could not be opened.")) + wxT("\n") + _("Operation cancelled!"), _("Copy To Clipboard Error"), wxOK | wxICON_ERROR);
			return wxString();
		}
	}
};

/// <summary>
/// UDK's Hex Editor class
/// </summary>
class UDKHexEditor : public UDKHexEditorControl
{
public:
	/**
	 * @brief Usual constructor for UDKHexEditor
	 */

	UDKHexEditor(wxWindow* parent,
		int id,
		wxStatusBar* statusbar = nullptr,
		DataInterpreter* interpreter = nullptr,
		InfoPanel* infopanel = nullptr,
		TagPanel* tagpanel = nullptr,
		DisassemblerPanel* dasmpanel = nullptr,
		CopyMaker* copy_mark = nullptr,
		wxFileName* myfile = nullptr,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);

	/**
	 * @brief Read the loaded buffer
	 *
	 * @see
	 */
	void ReadFromBuffer(uint64_t position, unsigned lenght, char* buffer, bool cursor_reset = true, bool paint = true);

	/**
	 * @brief Loads file from current page offset; refresh
	 *
	 * @see
	 */
	void Reload();

	/**
	 * @brief Loads file from specified page offset; refresh
	 *
	 * @see
	 */
	void LoadFromOffset(int64_t position, bool cursor_reset = false, bool paint = true, bool from_comparator = false);

	/**
	 * @brief Clears all kinds of controls
	 */
	void Clear(bool RePaint, bool cursor_reset);

	//----File Functions----//
	
	/**
	 * @brief Usual constructor for UDKHexEditor
	 * 
	 * This routine is called whilst hex editing.
	 * 
	 * @see DataInterpreter::OnTextEdit(wxKeyEvent& event)
	 */
	bool FileAddDiff(int64_t start_byte, const char* data, int64_t size, bool extension = false); //adds new node

	
	inline int64_t CursorOffset(void) 
	{
		return GetLocalInsertionPoint() + m_PageOffset;
	}

	/**
	 * @brief Returns position of Text Cursor
	 *
	 * The position where the edit needs be inserted
	 *
	 * @return position of text cursor
	 * @see 
	 */
	inline int GetLocalInsertionPoint()
	{
		return (FindFocus() == m_TextControl ? m_TextControl->GetInsertionPoint() * (GetCharToHexSize() / 2) : m_HexControl->GetInsertionPoint() / 2);
	}

	inline uint8_t GetCharToHexSize(void) 
	{
		if (m_TextControl->m_FontEnc == wxFONTENCODING_UTF16LE ||
			m_TextControl->m_FontEnc == wxFONTENCODING_UTF16BE)
			return 4;
		if (m_TextControl->m_FontEnc == wxFONTENCODING_UTF32LE ||
			m_TextControl->m_FontEnc == wxFONTENCODING_UTF32BE)
			return 8;
		return 2;
	}

	inline unsigned ByteCapacity(void)
	{
		return (m_HexControl->IsShown() ? m_HexControl->ByteCapacity() : m_TextControl->ByteCapacity());
	}

	int BytePerLine(void)
	{
		return wxMax(m_HexControl->IsShown() ? m_HexControl->BytePerLine() : m_TextControl->BytePerLine(), 1);
	}

	inline int GetByteCount(void) const
	{
		return (m_HexControl->IsShown() ? m_HexControl->GetByteCount() : m_TextControl->GetByteCount());
	}

	void PreparePaintTAGs();

	/**
	 * @brief Returns position of Text Cursor
	 *
	 * @return true on successful opening
	 * @see UDKHalo::OpenFile(wxFileName filename, bool openAtRight)
	 */
	bool FileOpen(wxFileName& filename);
public:
	/**
	 * @brief Reference to our dear hexeditor
	 */
	UDKHexEditor* m_ComparatorHexEditor;

	/**
	 * @brief Need to find out what this does
	 */
	int* m_ZebraStriping;

	bool m_ZebraEnable;

	int m_SectorSize;

	wxArrayUINT64 m_ProcessRAMMap;

	wxHugeScrollBar* m_OffsetScroll;

	Select* m_Select;

protected:
	/**
	 * @brief Holds current start offset of file
	 *
	 * Declared signed for error checking.
	 *
	 * @see DataInterpreter::OnTextEdit(wxKeyEvent& event)
	 */
	int64_t m_PageOffset;

	/**
	 * @brief The file currently being analyzed
	 */
	UDKFile* m_FileInMicroscope;

	/**
	 * @brief Reference to the pane holding file information
	 */
	InfoPanel* m_PanelOfInformation;

	DataInterpreter* m_DataInterpreter;

	// TagPanel *tagpanel;

	DisassemblerPanel *m_DisassemblerPanel;
	//copy_maker *copy_mark;

protected:
	void PaintSelection();

	void inline ClearPaint()
	{
		m_HexControl->ClearSelection();
		m_TextControl->ClearSelection();
	}
};

// Reroute all the function calls to UDKHalo
//class UDKHexEditorFrame : public UDKHalo
