#include "UDKControl.h"

#include <wx/encconv.h>
#include <wx/fontmap.h>

UDKControl::UDKControl(wxWindow* parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& validator) : wxScrolledWindow(parent, id,
	pos, size,
	wxSUNKEN_BORDER)
{
	m_HexDefaultAttr = wxTextAttr(
		wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT),
		wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT),
		wxFont(
			10,
			wxFONTFAMILY_MODERN,	// family
			wxFONTSTYLE_NORMAL,	// style
			wxFONTWEIGHT_BOLD,// weight
			true,				// underline
			wxT(""),			// facename
			wxFONTENCODING_CP437));// msdos encoding

	//Need to create object before Draw operation.
	m_ZebraStriping = new int;
	*m_ZebraStriping = -1;

	m_Hex2ColorMode = false;

	//Wayland hack
	wxString waylandStr;
		
	m_Waylander = wxGetEnv("WAYLAND_DISPLAY", &waylandStr);
		
	if (m_Waylander)
		std::cout << "Wayland detected. You could have cosmetic cursor issues." << std::endl;

	m_CtrlType = HexControl;
	m_DrawCharByChar = false;

	m_InternalBufferDC = nullptr;
	m_InternalBufferBMP = nullptr;

	m_HexFormat = wxT("xx ");
	m_Mycaret = NULL;

	//SetSelectionStyle(HexDefaultAttr);

	//ClearSelection(false);
	//SetDefaultStyle(HexDefaultAttr);

	m_Caret.x = m_Caret.y = m_Window.x = m_Window.y = 1;
	m_Margin.x = m_Margin.y = 0;

	m_LastRightClickPosition = wxPoint(0, 0);
	m_Select.m_Selected = false;

	//What is caret?
	//CreateCaret();

	MyConfigBase::Get()->Read(_T("Hex2ColorMode"), &m_Hex2ColorMode);
	//  ChangeSize();

	//wxCaret *caret = GetCaret();
	if (m_Mycaret)
	{
		m_Mycaret->Show(false);
	}
}

void UDKControl::RePaint(void)
{
	if (m_Waylander)
	{
		return this->Refresh();
	}

	m_PaintMutex.Lock();
	wxCaretSuspend cs(this);

	wxDC* dcTemp;// = UpdateDC(); //Prepare DC <----- need to write this long function

	if (dcTemp != NULL)
	{
		wxClientDC dc(this); //Not looks working on GraphicsContext
		///Directly creating contentx at dc creates flicker!
		//UpdateDC(&dc);

		//dc.DrawBitmap( *internalBufferBMP, this->GetSize().GetWidth(), this->GetSize().GetHeight() ); //This does NOT work
#ifdef WXOSX_CARBON  //wxCarbon needs +2 patch on both axis somehow.
		dc.Blit(2, 2, this->GetSize().GetWidth(), this->GetSize().GetHeight(), dcTemp, 0, 0, wxCOPY);
#else
		dc.Blit(0, 0, this->GetSize().GetWidth(), this->GetSize().GetHeight(), dcTemp, 0, 0, wxCOPY);
#endif //WXOSX_CARBON

#ifdef _Use_Graphics_Contex_
		wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
		if (gc) {
			//gc->DrawBitmap( *internalBufferBMP, 0.0, 0.0, dc.GetSize().GetWidth(), dc.GetSize().GetHeight());
			//gc->Flush();

			int TAC = TagArray.Count();
			if (TAC != 0)
				for (int i = 0; i < TAC; i++)
					TagPainterGC(gc, *TagArray.Item(i));

			if (select.selected)
				TagPainterGC(gc, select);
			delete gc;
		}
		else
			std::cout << " GraphicContext returs NULL!\n";
#else

#endif //_Use_Graphics_Contex_

		///delete dcTemp;
	}

	m_PaintMutex.Unlock();
}

wxString UDKOffsetControl::GetFormatString(bool minimal)
{
	wxString format;
	if (m_OffsetMode == 's')
	{
		unsigned sector_digit = 0;
		unsigned offset_digit = 0;
		if (!minimal)
		{
			while ((1 + m_OffsetLimit / m_SectorSize) > pow(10, ++sector_digit));
			while (m_SectorSize > pow(10, ++offset_digit));
		}
		format << wxT("%0") << sector_digit << wxT("llu:%0") << offset_digit << wxT("u");
		return format;
	}
	format << wxT("%0") << (minimal ? 0 : GetDigitCount()) << wxLongLongFmtSpec << wxChar(m_OffsetMode);

	if (m_OffsetMode == 'X')
	{
		format << wxChar('h');
	}
	else if (m_OffsetMode == 'o')
	{
		format << wxChar('o');
	}
	return format;
}

wxString UDKOffsetControl::GetFormatedOffsetString(uint64_t c_offset, bool minimal)
{
	if (m_OffsetMode == 's')
	{
		return wxString::Format(GetFormatString(minimal), uint64_t(c_offset / m_SectorSize), unsigned(c_offset % m_SectorSize));
	}
	return wxString::Format(GetFormatString(minimal), c_offset);
}

unsigned UDKOffsetControl::GetDigitCount(void)
{
	m_DigitCount = 0;
	int base = 0;
	switch (m_OffsetMode)
	{
	case 'u':
		base = 10;
		break;
	case 'X':
		base = 16;
		break;
	case 'o':
		base = 8;
		break;
	case 's':
		base = 10;
		break;
	}
	if (m_OffsetMode == 's')
	{
		int digit_count2 = 0;
		while (1 + (m_OffsetLimit / m_SectorSize) > pow(base, ++m_DigitCount));
		while (m_SectorSize > pow(base, ++digit_count2));
		m_DigitCount += digit_count2;
	}

	while (m_OffsetLimit > pow(base, ++m_DigitCount));
	if (m_DigitCount < 6)
	{
		m_DigitCount = 6;
	}

	return m_DigitCount;
}

unsigned UDKOffsetControl::GetLineSize(void)
{
	unsigned line_size = GetDigitCount();
	if (m_OffsetMode == 'X' || m_OffsetMode == 'o')
	{
		line_size++;
	}
	return line_size;
}

///------HEXOFFSETCTRL-----///
void UDKOffsetControl::SetValue(uint64_t position)
{
	SetValue(position, m_BytePerLine);
}

void UDKOffsetControl::SetValue(uint64_t position, int byteperline)
{
	m_OffsetPosition = position;
	m_BytePerLine = byteperline;
	m_text.Clear();

	wxString format = GetFormatString();
	//not wxULongLong_t ull due no wxULongLongFmtSpec
	//wxLongLong_t ull = ( offset_position );
	uint64_t ull = (m_OffsetPosition);
	if (m_OffsetMode == 's')
	{
		//Sector Indicator!
		for (int i = 0; i < LineCount(); i++) {
			m_text << wxString::Format(format, int64_t(ull / m_SectorSize), unsigned(ull % m_SectorSize));
			ull += m_BytePerLine;
		}
	}
	else
		for (int i = 0; i < LineCount(); i++)
		{
			m_text << wxString::Format(format, ull);
			ull += m_BytePerLine;
		}
	RePaint();
}

void UDKOffsetControl::OnMouseRight(wxMouseEvent& event)
{
	event.Skip();
	m_LastRightClickPosition = event.GetPosition();
	
	ShowContextMenu(m_LastRightClickPosition);
}

int UDKControl::PixelCoordToInternalPosition(wxPoint mouse)
{
	mouse = PixelCoordToInternalCoord(mouse);
	return (mouse.x - xCountDenied(mouse.x) + mouse.y * CharacterPerLine());
}

wxPoint UDKControl::PixelCoordToInternalCoord(wxPoint mouse)
{
	mouse.x = (mouse.x < 0 ? 0 : mouse.x);
	mouse.x = (mouse.x > m_CharSize.x * m_Window.x ? m_CharSize.x * m_Window.x - 1 : mouse.x);
	mouse.y = (mouse.y < 0 ? 0 : mouse.y);
	mouse.y = (mouse.y > m_CharSize.y * m_Window.y ? m_CharSize.y * m_Window.y - 1 : mouse.y);
	int x = (mouse.x - m_Margin.x) / m_CharSize.x;
	int y = (mouse.y - m_Margin.y) / m_CharSize.y;
	return wxPoint(x, y);
}

int UDKControl::CharacterPerLine(bool NoCache)
{
	//Without spaces
	if (!NoCache)
	{
		return CPL;
	}
	int avoid = 0;
	for (int x = 0; x < m_Window.x; x++)
		avoid += m_IsDeniedCache[x];
	CPL = m_Window.x - avoid;
	//std::cout << "CPL: " << CPL << std::endl;
	return (m_Window.x - avoid);
}

int UDKControl::xCountDenied(int x)
{
	//Counts denied character locations (spaces) on given x coordination
	for (int i = 0, denied = 0; i < m_Window.x; i++)
	{
		if (IsDenied(i))
		{
			denied++;
		}
		if (i == x)
		{
			return denied;
		}
	}
	return -1;
}

void UDKControl::ShowContextMenu(wxPoint pos)
{
	wxMenu menu;

	unsigned TagPosition = PixelCoordToInternalPosition(pos);
	TagElement* TAG;
	for (unsigned i = 0; i < m_TagArray.Count(); i++) {
		TAG = m_TagArray.Item(i);
		if ((TagPosition >= TAG->m_Start) && (TagPosition < TAG->m_End))
		{
			//end not included!
			menu.Append(__idTagEdit__, _T("Tag Edit"));
			break;
		}
	}

	if (m_Select.m_Selected)
	{
		menu.Append(__idTagAddSelect__, _T("Tag Selection"));
	}

	//  menu.AppendSeparator();
	PopupMenu(&menu, pos);
	// test for destroying items in popup menus

#if 0 // doesn't work in wxGTK!
	menu.Destroy(Menu_Popup_Submenu);
	PopupMenu(&menu, event.GetX(), event.GetY());
#endif // 0
}

void UDKOffsetControl::OnMouseLeft(wxMouseEvent& event)
{
	wxPoint p = PixelCoordToInternalCoord(event.GetPosition());
	uint64_t address = m_OffsetPosition + p.y * m_BytePerLine;
	wxString adr;
	if (m_OffsetMode == 's')
	{
		adr = wxString::Format(GetFormatString(), (1 + address / m_SectorSize), address % m_SectorSize);
	}
	else
	{
		adr = wxString::Format(GetFormatString(), address);
	}

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->Clear();
		if (!wxTheClipboard->SetData(new wxTextDataObject(adr)))
		{
			wxBell();
		}
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}
}

inline bool UDKControl::IsDenied(int x)
{
	// State Of The Art :) Hex plotter function by idents avoiding some X axes :)
	return m_IsDeniedCache[x];
}

inline bool UDKControl::IsDenied_NoCache(int x)
{
	// State Of The Art :) Hex plotter function by idents avoiding some X axes :)
	//		x%=m_Window.x;						// Discarding y axis noise
	if (1)
	{
		//EXPERIMENTAL
		if (((m_Window.x - 1) % m_HexFormat.Len() == 0)	// For avoid hex divorcings
			&& (x == m_Window.x - 1))
		{
			return true;
		}
		return m_HexFormat[x % (m_HexFormat.Len())] == ' ';
	}

	if (((m_Window.x - 1) % 3 == 0)		// For avoid hex divorcings
		&& (x == m_Window.x - 1))
	{
		return true;
	}
	//	if( x == 3*8 )
	//		return true;
	return !((x + 1) % 3);				// Byte coupling
}

bool UDKControl::IsAllowedChar(const char& chr)
{
	return isxdigit(chr);
}

UDKControl::~UDKControl()
{
	Clear();
	wxCaretSuspend cs(this);
}

void UDKControl::Clear(bool RePaint, bool cursor_reset)
{
	m_Text.Clear();

	if (cursor_reset)
	{
		SetInsertionPoint(0);
	}

	OnTagHideAll();
	ClearSelection(RePaint);
	
	WX_CLEAR_ARRAY(m_TagArray);
}

void UDKControl::OnTagHideAll(void)
{
	for (unsigned i = 0; i < m_TagArray.Count(); i++)
	{
		m_TagArray.Item(i)->Hide();
	}
}

void UDKControl::ClearSelection(bool repaint)
{
	m_Select.m_Start = 0;
	m_Select.m_End = 0;
	m_Select.m_Selected = false;
	if (repaint)
	{
		RePaint();
	}
}

void UDKControl::SetInsertionPoint(unsigned int pos)
{
	if (pos > m_Text.Length())
	{
		pos = m_Text.Length();
	}

	pos = ToVisiblePosition(pos);
	MoveCaret(wxPoint(pos % m_Window.x, pos / m_Window.x));
}

int UDKControl::ToVisiblePosition(int InternalPosition)
{
	// I mean for this string on hex editor  "00 FC 05 C[C]" , while [] is cursor
	if (CharacterPerLine() == 0)
	{
		return 0;					// Visible position is 8 but internal position is 11
	}
	
	int y = InternalPosition / CharacterPerLine();
	int x = InternalPosition - y * CharacterPerLine();
	
	for (int i = 0, denied = 0; i < m_Window.x; i++)
	{
		if (IsDenied(i)) denied++;
		if (i - denied == x)
		{
			return (i + y * m_Window.x);
		}
	}

	return 0;
}

void UDKControl::MoveCaret(wxPoint p)
{
#ifdef _DEBUG_CARET_
	std::cout << "MoveCaret(wxPoint) Coordinate X:Y = " << p.x << " " << p.y << std::endl;
#endif
	m_Caret = p;
	DoMoveCaret();
}

void UDKControl::DoMoveCaret()
{
	wxCaret* caret = GetCaret();
#ifdef _DEBUG_CARET_
	std::cout << "Caret = 0x" << (intptr_t)caret << " - mycaret= 0x" << (intptr_t)mycaret << "m_charSize.x" << m_CharSize.x << std::endl;
#endif
	if (caret)
	{
		caret->Move(m_Margin.x + m_Caret.x * m_CharSize.x,
			m_Margin.x + m_Caret.y * m_CharSize.y);
	}
}