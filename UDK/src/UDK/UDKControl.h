/*
 *   ---------------------
 *  |  UDKControl.h
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

#include <stdint.h>
#include <wx/defs.h>
#include <wx/buffer.h>
#include <wx/graphics.h>
#include <wx/textctrl.h>
#include <wx/caret.h>
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>
#include <wx/window.h>
#include <wx/popupwin.h>

#define __idTagAddSelect__ 1500
#define __idTagEdit__ 1501
#define __idOffsetHex__ 1502

wxArrayString GetSupportedEncodings();
inline wxChar CP473toUnicode(unsigned char ch);
inline wxString CP473toUnicode(wxString& line);
int atoh(const char);

// Need to create seperate file
class TagElement
{
public:
	void Hide(void)
	{
		if (m_Visible)
		{
#ifdef _DEBUG_TAG_
			std::cout << "Hide tag element " << this << std::endl;
#endif
			m_Visible = false;
			m_WxP->Hide();
			m_WxP->Destroy();
			m_WxP = NULL;
		}
	}

public:
	uint64_t				m_Start;
	uint64_t				m_End;
	bool					m_Visible;
	wxPopupWindow*			m_WxP;
};

WX_DEFINE_ARRAY(TagElement*, ArrayOfTAG);

class UDKControl : public wxScrolledWindow
{
public:
	UDKControl(wxWindow* parent)
	{}

	UDKControl(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	~UDKControl();

	wxChar CharAt(unsigned offset);
	int LineCount(void)
	{
		return m_Window.y;
	}
	void RePaint(void);
	void MoveCaret(wxPoint p);
	void DoMoveCaret();		// move the caret to m_Caret.x, m_Caret.y

	void OnTagHideAll(void);
	void ClearSelection(bool RePaint = true);

	// Movement Support
	virtual int CharacterPerLine(bool NoCache = false);

	virtual int PixelCoordToInternalPosition(wxPoint mouse);
	wxPoint PixelCoordToInternalCoord(wxPoint mouse);

	// Shaper Classes, All other classes has to be depended to this function for proper action!
	bool m_IsDeniedCache[1024];//cache, Enough for these days hehe
	int CPL;
	virtual bool IsDenied()
	{
		return IsDenied(m_Caret.x);
	}
	virtual bool IsDenied(int x);
	virtual bool IsDenied_NoCache(int x);
	virtual bool IsAllowedChar(const char& chr);
	//virtual	const char Filter(const char& ch);
	int xCountDenied(int x);
	void Clear(bool ClearDC = true, bool cursor_reset = true);
	virtual void SetInsertionPoint(unsigned int pos);
	virtual int ToVisiblePosition(int InternalPosition);

public:
	struct Selector : public TagElement 
	{
		//select
		bool m_Selected;		//select available variable
	} m_Select;

	// TAG Support and Selection
	ArrayOfTAG m_TagArray;

protected:
	wxPoint				m_Margin;	// the margin around the text (looks nicer)
	wxPoint				m_Caret;	// position (in text coords) of the caret
	wxPoint				m_Window;	// the size (in text coords) of the window
	wxString			m_text;
	wxTextAttr			m_HexDefaultAttr;
	wxMutex				m_PaintMutex;
	wxPoint				m_LastRightClickPosition;	//Holds last right click for TagEdit function
	wxString			m_HexFormat;
	wxString			m_Text;
	wxSize				m_CharSize;	// size (in pixels) of one character

protected:
	void ShowContextMenu(wxPoint pos);

public:
	//inline void DrawSeperationLineAfterChar(wxDC* DC, int offset);
	//void OnTagHideAll(void);
	bool* m_TagMutex;
	int* m_ZebraStriping;
	bool m_Hex2ColorMode;
	bool m_Waylander;
	wxMemoryDC* m_InternalBufferDC;
	wxBitmap* m_InternalBufferBMP;
	bool		m_DrawCharByChar;

	enum CtrlTypes 
	{
		HexControl, 
		TextControl, 
		OffsetControl 
	};
	CtrlTypes m_CtrlType;

	// Caret Movement
	wxCaret* m_Mycaret;
};

///<summary>
/// Wrapper for Portable vs Registry configbase.\n
/// if there are wxHexEditor.cfg file exits on current path, wxHexEditor switches to portable version.
/// </summary>
class MyConfigBase
{
public:
	static wxConfigBase* Get()
	{
		static wxFileConfig* AppConfigFile = new wxFileConfig("", "", "wxHexEditor.cfg", "", wxCONFIG_USE_RELATIVE_PATH);
		if (wxFileExists("wxHexEditor.cfg"))
		{
			return AppConfigFile;
		}
		else
		{
			return wxConfigBase::Get();
		}
	}
};

class UDKOffsetControl : public UDKControl
{
public:
	UDKOffsetControl(wxWindow* parent) : UDKControl(parent)
	{
		m_OffsetMode = 'u';
		m_OffsetPosition = 0;
		m_DigitCount = 6;
	}

	UDKOffsetControl(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator) :
		UDKControl(parent, id, value, pos, size, style, validator)
	{
		wxCaret* caret = GetCaret();
		if (caret)
			GetCaret()->Hide();
		SetCaret(NULL);

		//offset_mode='u';
		m_CtrlType = OffsetControl;
		m_OffsetMode = MyConfigBase::Get()->Read(_T("LastOffsetMode"), wxT("u"))[0];
		if (m_OffsetMode == 's')	// No force to sector mode at startup.
		{
			m_OffsetMode = 'u';
		}

		m_OffsetPosition = 0;
		m_DigitCount = 6;
	};

	wxString GetFormatString(bool minimal = false);
	wxString GetFormatedOffsetString(uint64_t c_offset, bool minimal = false);
	void SetOffsetLimit(uint64_t max_offset)
	{
		m_OffsetLimit = max_offset;
	}
	unsigned GetDigitCount(void);
	unsigned GetLineSize(void);  //Digit count plus Formating chars like h,o if required
	inline bool IsDenied()
	{
		return false;
	}
	inline bool IsDenied(int x)
	{
		return false;
	}
	int ToVisiblePosition(int InternalPosition)
	{
		return InternalPosition;
	}
	int ToInternalPosition(int VisiblePosition)
	{
		return VisiblePosition;
	}
	void SetValue(uint64_t position);
	void SetValue(uint64_t position, int byteperline);
	void OnMouseRight(wxMouseEvent& event);
	void OnMouseLeft(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event)
	{
		event.Skip(false);
	}
	char m_OffsetMode;
	uint64_t m_OffsetPosition;
	int m_BytePerLine;
	int m_SectorSize;

private:
	uint64_t m_OffsetLimit;
	unsigned m_DigitCount;
	inline void DrawCursorShadow(wxDC* dcTemp) {}
};