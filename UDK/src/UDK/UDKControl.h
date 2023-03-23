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
 *   March, 2023: Bringing in code from wxHexEditor
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
#include <wx/colourdata.h>

class UDKOffsetControl;
class UDKElementControl;

class UDKTextControl;

#define ID_DEFAULT wxID_ANY // Default
#define ID_HEXBOX 1000
#define ID_TEXTBOX 1001

/// <summary>
/// The controller class for UDKHalo frame (written in UDKChief.h)
/// </summary>
class UDKHexEditorControl : public wxPanel
{
private:

protected:
	wxStaticText* m_StaticOffset;
	wxStaticText* m_StaticAddress;
	wxStaticText* m_StaticByteview;
	wxStaticText* m_StaticNull;
	UDKElementControl* m_HexControl;
	UDKOffsetControl* m_OffsetControl;
	UDKTextControl* m_TextControl;

	// Virtual event handlers, overide them in your derived class
	virtual void OnKeyboardChar(wxKeyEvent& event) { event.Skip(); }
	virtual void OnResize(wxSizeEvent& event) { event.Skip(); }
	virtual void OnOffsetScroll(wxScrollEvent& event) { event.Skip(); }

public:
	wxScrollBar* m_OffsetScrollReal;

	UDKHexEditorControl(wxWindow* parent, wxWindowID id = ID_DEFAULT, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxTAB_TRAVERSAL);
	~UDKHexEditorControl();
};

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
	wxPopupWindow*				m_WxP;
	wxColourData				m_NoteColourData;
	wxColourData				m_FontColorData;
};

WX_DEFINE_ARRAY(TagElement*, ArrayOfTAG);

enum ControlTypes
{
	HexControl,
	TextControl,
	OffsetControl
};

/// <summary>
/// The UDKElementControl class
/// </summary>
class UDKElementControl : public wxScrolledWindow
{
public:
	UDKElementControl(wxWindow* parent)
	{}

	UDKElementControl(wxWindow* parent,
		wxWindowID id,
		const wxString& value = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator);

	~UDKElementControl();

	inline wxChar CharAt(unsigned offset)
	{
		if(offset >= m_Text.Len())
		{
			//std::cout << "Buff lower for offset : " << offset << std::endl;
			return 0;
		}
		return m_Text.GetChar(offset);
	}
	int LineCount(void)
	{
		return m_Window.y;
	}
	void RePaint(void);
	void MoveCaret(wxPoint p);
	void DoMoveCaret();		// move the caret to m_Caret.x, m_Caret.y

	void OnTagHideAll(void);

	/**
	 * @brief Update geometry and relevant variables
	 *
	 *
	 */
	virtual void ChangeSize();

	/**
	 * @brief Sets the displayable character style
	 *
	 * Sets the m_CharSize.x and .y values
	 *
	 * @see
	 */
	virtual void SetDefaultStyle(wxTextAttr& new_attr);

	// Movement Support
	virtual int CharacterPerLine(bool NoCache = false);

	virtual int PixelCoordToInternalPosition(wxPoint mouse);
	wxPoint PixelCoordToInternalCoord(wxPoint mouse);

	// Shaper Classes, all other classes have to depend on this function for proper action!
	bool m_IsDeniedCache[1024];//cache, Enough for these days hehe

	/**
	 * @brief Characters per line
	 *
	 * @see UDKElementControl::CharacterPerLine(bool NoCache)
	 */
	uint32_t m_CPL;
	virtual bool IsDenied()
	{
		return IsDenied(m_Caret.x);
	}
	virtual bool IsDenied(int x);

	/**
	 * @brief IsDenied_NoCache
	 * @param x
	 * @return
	 */
	virtual bool IsDenied_NoCache(int x);
	virtual bool IsAllowedChar(const char& chr);
	int xCountDenied(int x);
	void Clear(bool ClearDC = true, bool cursor_reset = true);
	virtual void SetInsertionPoint(size_t pos);
	virtual size_t ToVisiblePosition(size_t InternalPosition);

	virtual int GetByteCount(void)
	{
		return m_Text.Length() / 2;
	}

	int GetInsertionPoint(void)
	{
		return (m_Caret.x - xCountDenied(m_Caret.x)) + CharacterPerLine() * m_Caret.y;
	}

	virtual int ByteCapacity(void)
	{
		std::cout << "m_Window.y = " << m_Window.y << std::endl;
		return m_Window.y * BytePerLine();
	}

	virtual int BytePerLine(void)
	{
		std::cout << "CharacterPerLine: " << CharacterPerLine() / 2 << std::endl;
		return CharacterPerLine() / 2;
	}

	void SetBinValue(wxString buffer, bool repaint = true);
	void SetBinValue(char* buffer, int byte_count, bool repaint = true);

	void SetSelection(unsigned start, unsigned end);
	void ClearSelection(bool RePaint = true);

	static wxMemoryBuffer HexToBin(const wxString& HexValue);

	// Movement
	virtual void DrawCursorShadow(wxDC*);

	inline void DrawSeperationLineAfterChar(wxDC* DC, int offset);

	wxPoint InternalPositionToVisibleCoord(int position);

#ifdef _USE_GRAPHICS_CONTEXT_
	/**
	 * @brief For painting the context supplied by device context
	 *
	 * Once the device context, wxMemoryDC here, is complete, this routine \n
	 * is called to render using a graphics context
	 *
	 * @see UDKElementControl::OnPaint(wxPaintEvent &WXUNUSED(event))
	 */
	virtual void TagPainterGC(wxGraphicsContext* gc, TagElement& TG);
#endif // _USE_GRAPHICS_CONTEXT_

public:
	struct Selector : public TagElement
	{
		//select
		bool m_Selected;		//select available variable
	} m_Select;

	// TAG Support and Selection
	ArrayOfTAG		m_TagArray;
	wxArrayInt		m_ThinSeparationLines;

protected:
	/**
	 * @brief The margin around the text (looks nicer)
	 */
	wxPoint				m_Margin;
	wxPoint				m_Caret;	// position (in text coords) of the caret

	/**
	 * @brief The size of window in text coordinates
	 *
	 * ::x and ::y are the counters of characters in x and y directions (axis)
	 *
	 * @see UDKElementControl::CharacterPerLine(bool NoCache)
	 */
	wxPoint				m_Window;

	wxTextAttr			m_HexDefaultAttr;
	wxMutex				m_PaintMutex;
	wxPoint				m_LastRightClickPosition;	//Holds last right click for TagEdit function
	wxString			m_HexFormat;


	/**
	 * @brief The text form of hexdata
	 *
	 * The text incarnation of hex data ? obtained from file
	 *
	 * @see UDKElementControl::SetBinValue(char* buffer, int byte_count, bool repaint)
	 */

	wxString			m_Text;

	/**
	 * @brief size (in pixels) of one character
	 *
	 * You might wonder why is this not wxPoint and someone from Turkey \n
	 * should have the answer.
	 */
	wxSize				m_CharSize;

	ControlTypes			m_ControlType;
protected:
	void ShowContextMenu(wxPoint pos);

	/**
	 * @brief Updates the device context
	 *
	 * This routine updates the device context onto which images and (or) \n
	 * text can be drawn.\n
	 *
	 * For more (basic and verbose) information I'd go to \n
	 * https://www.informit.com/articles/article.aspx?p=405047
	 *
	 * @see UDKElementControl::OnPaint(wxPaintEvent &WXUNUSED(event))
	 * @return The device context to draw
	 */
	wxDC* UpdateDC(wxDC* dc = nullptr);

	/**
	 * @brief What to do with the event ChangeSize()
	 *
	 * @see UDKElementControl::ChangeSize()
	 */
	void OnSize(wxSizeEvent &event);

	/**
	 * @brief What to do with the event RePaint()?
	 *
	 *
	 *
	 * @see UDKElementControl::RePaint(void)
	 */
	void OnPaint(wxPaintEvent &event);
	DECLARE_EVENT_TABLE();

	/**
	 * @brief Creates the device context
	 *
	 * Creates a memory device context provides a means to draw graphics \n
	 * onto a bitmap. Fills m_InternalBufferDC and m_InternalBufferBMP
	 *
	 * @see UDKElementControl::ChangeSize()
	 */
	wxMemoryDC* CreateDC();

public:
	//inline void DrawSeperationLineAfterChar(wxDC* DC, int offset);
	//void OnTagHideAll(void);
	bool*					m_TagMutex;
	int*					m_ZebraStriping;
	bool					m_Hex2ColorMode;
	bool					m_Waylander;

	/**
	 * @brief reference to internal buffer wxMemoryDC
	 *
	 * A memory device context provides a means to draw graphics onto a bitmap
	 *
	 * @see UDKElementControl::CreateDC()
	 */
	wxMemoryDC*				m_InternalBufferDC;

	/**
	 * @brief reference to internal buffer wxMemoryDC
	 *
	 * This stores the reference to the encapsulation of the concept of \n
	 * a platform-dependent bitmap, either monochrome or colour or colour \n
	 * with alpha channel support
	 *
	 * @see UDKElementControl::CreateDC()
	 */
	wxBitmap*				m_InternalBufferBMP;
	bool					m_DrawCharByChar;

	// Caret Movement
	/**
	 * brief A caret is a blinking cursor showing the position where the typed text will appear
	 *
	 */
	wxCaret*				m_Mycaret;
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

class UDKOffsetControl : public UDKElementControl
{
public:
	UDKOffsetControl(wxWindow* parent) : UDKElementControl(parent)
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
		UDKElementControl(parent, id, value, pos, size, style, validator)
	{
		wxCaret* caret = GetCaret();
		if (caret)
			GetCaret()->Hide();
		SetCaret(NULL);

		//offset_mode='u';
		m_ControlType = OffsetControl;
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
	int m_SectorSize;// don't do this? Write appropriate getter and setter

private:
	uint64_t m_OffsetLimit;
	unsigned m_DigitCount;
	inline void DrawCursorShadow(wxDC* dcTemp) {}
};

class UDKTextControl : public UDKElementControl
{
public:
	//		wxHexTextCtrl():wxHexCtrl(){}
	UDKTextControl(wxWindow * parent) : UDKElementControl(parent)
	{
		m_ControlType = TextControl;
	}
	UDKTextControl(wxWindow * parent,
		wxWindowID id,
		const wxString & value = wxEmptyString,
		const wxPoint & pos = wxDefaultPosition,
		const wxSize & size = wxDefaultSize,
		long style = 0,
		const wxValidator & validator = wxDefaultValidator) :
		UDKElementControl(parent, id, value, pos, size, style, validator)
	{
		m_ControlType = TextControl;
		wxWindow::SetCursor(wxCURSOR_CHAR);

		m_FontEnc = wxFONTENCODING_ALTERNATIVE;

		wxString cp;
		MyConfigBase::Get()->Read(_T("CharacterEncoding"), &cp, wxT("DOS CP437"));
		PrepareCodepageTable(cp);
	};

	//wxArrayString GetSupportedEncodings(void);
	wxString PrepareCodepageTable(wxString);
	inline bool IsDenied() override
	{
		return false;
	}
	inline bool IsDenied(int x) override
	{
		return false;
	}
	inline int CharacterPerLine(void)
	{
		return m_Window.x;
	}
	inline int BytePerLine(void) override
	{
		return CharacterPerLine();
	}
	inline int GetByteCount(void) override
	{
		return m_Text.Length();
	}
	//void Replace(unsigned text_location, const wxChar & value, bool paint);
	//void ChangeValue(const wxString & value, bool paint);
	//void SetBinValue(char* buffer, int len, bool paint);
	//void SetDefaultStyle(wxTextAttr & new_attr);		//For caret diet (to 1 pixel)
	//int PixelCoordToInternalPosition(wxPoint mouse);
	int ToVisiblePosition(int InternalPosition)
	{
		return InternalPosition;
	}
	int ToInternalPosition(int VisiblePosition)
	{
		return VisiblePosition;
	}
	//		bool IsAllowedChar(const unsigned char& chr);
	//int GetInsertionPoint(void);
	//void SetInsertionPoint(unsigned int pos);

	/**
	 * @brief Evaluate size relevant variables
	 *
	 */

	//void ChangeSize();
	//wxChar Filter(const unsigned char& chr);
	//wxString FilterMBBuffer(const char* str, int len, int fontenc);
	//virtual void DrawCursorShadow(wxDC * dcTemp);

public:
	wxString m_CodepageTable;
	wxString m_Codepage;

	wxFontEncoding m_FontEnc;
};
