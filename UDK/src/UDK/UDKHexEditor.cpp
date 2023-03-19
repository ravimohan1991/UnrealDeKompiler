#include "UDKHexEditor.h"
#include "UDKFile.h"

extern int FakeBlockSize;

UDKHexEditor::UDKHexEditor(wxWindow* parent,
						int id,
						wxStatusBar *statbar_,
						DataInterpreter *interpreter_,
						InfoPanel *infopanel_,
						TagPanel *tagpanel_,
						DisassemblerPanel *dasmpanel_,
						CopyMaker *copy_mark_,
						wxFileName* myfilename_,
						const wxPoint& pos,
						const wxSize& size,
						long style ):
	UDKHexEditorControl(parent, id, pos, size, wxTAB_TRAVERSAL),
	//statusbar(statbar_),
	m_DataInterpreter(interpreter_),
	m_PanelOfInformation(infopanel_),
	//tagpanel(tagpanel_),
	m_DisassemblerPanel(dasmpanel_)
	//copy_mark(copy_mark_)
{
	m_ComparatorHexEditor = nullptr;

	// Here, code praying to the GOD for protecting our open file from wxHexEditor's bugs and other things.
	// This is really crucial step! Be adviced to not remove it, even if you don't believer.
	printf("Rahman ve Rahim olan Allah'ın adıyla.\n");

	m_FileInMicroscope = nullptr;

#ifndef DO_NOT_USE_THREAD_FOR_SCROLL
	//myscrollthread = NULL;
#endif

	if( myfilename_ != NULL )
	{
		if(!FileOpen(*myfilename_ ))
		{
		}
	}

	//offset_scroll->Enable( true );
	//Dynamic_Connector();
	//BlockSelectOffset = -1;
	//MouseCapture = false;
}

bool UDKHexEditor::FileAddDiff(int64_t start_byte, const char* data, int64_t size, bool injection)
{
	if (m_FileInMicroscope->m_FileLock)
	{
		wxMessageBox(_("File is locked for edit."), _("Error"), wxOK | wxICON_ERROR);
		return false;
	}
	else
	{
		return m_FileInMicroscope->Add(start_byte, data, size, injection);
	}
}

void UDKHexEditor::Reload()
{
	LoadFromOffset(m_PageOffset, false, true, false);
}

void UDKHexEditor::LoadFromOffset(int64_t position, bool cursor_reset, bool paint, bool from_comparator)
{
#ifdef _DEBUG_FILE_
	std::cout << "\nLoadFromOffset() : " << position << std::endl;
#endif

	//For file compare mode
	if (m_ComparatorHexEditor != NULL && !from_comparator)
	{
		m_ComparatorHexEditor->LoadFromOffset(position, cursor_reset, true, true);
	}

	m_FileInMicroscope->Seek(position, wxFromStart);
	char* buffer = new char[ByteCapacity()];
	int readedbytes = m_FileInMicroscope->Read(buffer, ByteCapacity());

	if (readedbytes == -1) {
		wxMessageBox(_("File cannot read!"), _("Error"), wxOK | wxICON_ERROR);
		delete[] buffer;
		return;
	}
	ReadFromBuffer(position, readedbytes, buffer, cursor_reset, paint);
	delete[] buffer;
}

void UDKHexEditor::Clear(bool RePaint, bool cursor_reset)
{
	m_HexControl->Clear(RePaint, cursor_reset);
	m_TextControl->Clear(RePaint, cursor_reset);
	m_OffsetControl->Clear(RePaint, cursor_reset);
}

bool UDKHexEditor::FileOpen(wxFileName& myfilename)
{
	if(m_FileInMicroscope != nullptr)
	{
		wxLogError(_("Critical Error. File pointer is not empty!"));
		return false;
	}

	//Windows Device Loader
#ifdef __WXMSW__
	//if File is Windows device file! Let pass it and process under FAM
	if( m_FileInMicroscope.GetFullPath().StartsWith( wxT(".:"))
		  || m_FileInMicroscope.GetFullPath().StartsWith( wxT("\\Device\\Harddisk") ))
	{
		m_FileInMicroscope = new FAL( myfilename ); //OpenDevice
		if(m_FileInMicroscope->IsOpened())
		{
#ifndef DO_NOT_USE_THREAD_FOR_SCROLL
			//myscrollthread = new scrollthread(0,this);
#endif
//			copy_mark = new copy_maker();
			offset_ctrl->SetOffsetLimit( FileLength() );
			sector_size = FDtoBlockSize( GetFD() );//myfile->GetBlockSize();
			LoadFromOffset(0, true);
			SetLocalHexInsertionPoint(0);
			return true;
		}
		else
		{
			wxMessageBox(_("File cannot open."),_("Error"), wxOK|wxICON_ERROR, this);
			return false;
		}
	}
	else
#endif
	if (myfilename.GetSize( ) < 50*MB && myfilename.IsFileWritable())
	{
		m_FileInMicroscope = new UDKFile(myfilename, FileAccessMode::ReadWrite, 0);
	}
	else
	{
		m_FileInMicroscope = new UDKFile(myfilename, FileAccessMode::ReadOnly, 0);
	}

	if(m_FileInMicroscope->IsOpened())
	{
#ifndef DO_NOT_USE_THREAD_FOR_SCROLL
		//myscrollthread = new scrollthread(0,this);
#endif
//		copy_mark = new copy_maker();

		if(m_FileInMicroscope->IsProcess())
		{
			///autogenerate Memory Map as Tags...
#ifdef __WXGTK__
			//offset_scroll->Enable(false);
			std::cout << "PID MAPS loading..." << std::endl;
			wxString command( wxT("cat /proc/") );
			command << myfile->GetPID() << wxT("/maps");
			std::cout << command.ToAscii() << std::endl;
			wxArrayString output;

			wxExecute( command, output);
			//output has Count() lines process it

			for( unsigned i=0; i < output.Count() ; i++) {
				TagElement *tmp = new TagElement();
				long long unsigned int x;
				output[i].BeforeFirst('-').ToULongLong(&x, 16);
				tmp->start = x;
				tmp->end = x;
				ProcessRAMMap.Add(x);
				output[i].AfterFirst('-').BeforeFirst(' ').ToULongLong(&x,16);
				ProcessRAMMap.Add(x);
				tmp->tag = output[i].AfterLast( wxT(' '));
				tmp->FontClrData.SetColour( *wxBLACK );
				tmp->NoteClrData.SetColour( *wxCYAN );
				MainTagArray.Add(tmp);
				}
#endif
#ifdef __WXMSW__
			MEMORY_BASIC_INFORMATION mymeminfo;
			const uint8_t *addr, *addr_old;
			addr=addr_old=0;
			wxChar bfrx[200];
			memset( bfrx, 0, 200);
			wxString name, name_old;
			while( true ) {
				if( addr_old > addr )
					break;
				addr_old=addr;
				VirtualQueryEx( myfile->GetHandle(), addr, &mymeminfo, sizeof( MEMORY_BASIC_INFORMATION));
				GetMappedFileName(myfile->GetHandle(), (LPVOID)addr, bfrx, 200);
				name=wxString(bfrx);
				/*
				std::cout << "Addr :" << addr << " - " << mymeminfo.RegionSize << "   \t State: " << \
				(mymeminfo.State & MEM_COMMIT  ? "Commit" : "") << \
				(mymeminfo.State & MEM_FREE  ? "Free" : "") << \
				(mymeminfo.State & MEM_RESERVE  ? "Reserve" : "") << \
				"  Type: " << \
				(mymeminfo.Type & MEM_IMAGE  ? "Image   " : "") << \
				(mymeminfo.Type & MEM_IMAGE  ? "Mapped  " : "") << \
				(mymeminfo.Type & MEM_IMAGE  ? "Private " : "") << \
				"\tName: " << wxString(bfrx).c_str() << std::endl;
				*/
				if( name!=name_old) {
					name_old=name;
					TagElement *tmp = new TagElement();
					long long unsigned int x;
					tmp->start = reinterpret_cast<uint64_t>(addr);

					//tmp->end = reinterpret_cast<uint64_t>(addr+mymeminfo.RegionSize);
					//Just for indicate start of the DLL.
					tmp->end = reinterpret_cast<uint64_t>(addr+1);

					ProcessRAMMap.Add(x);
					tmp->tag = name;

					tmp->FontClrData.SetColour( *wxBLACK );
					tmp->NoteClrData.SetColour( *wxCYAN );
					MainTagArray.Add(tmp);
					}
				addr+=mymeminfo.RegionSize;
				}
#endif
			}

		//LoadTAGS( myfilename.GetFullPath().Append(wxT(".tags")) ); //Load tags to wxHexEditorCtrl

		//tagpanel->Set(MainTagArray); //Sets Tags to Tag panel

		//if(MainTagArray.Count() > 0) {
			//TODO This tagpanel->Show() code doesn't working good
			//tagpanel->Show();
		//	}

		m_OffsetControl->SetOffsetLimit(FileLength());
		m_SectorSize = m_FileInMicroscope->GetBlockSize();
		LoadFromOffset(0, true);
		SetLocalHexInsertionPoint(0);
		return true;
	}
	else
	{
		///Handled on FAM Layer...
		///wxMessageBox(_("File cannot open."),_("Error"), wxOK|wxICON_ERROR, this);
		return false;
	}
}

void UDKHexEditor::ReadFromBuffer(uint64_t position, unsigned lenght, char* buffer, bool cursor_reset, bool paint) 
{
	if (lenght == 4294967295LL)
	{
		std::cout << "Buffer has no data!" << std::endl;
		return;
	}

	static wxMutex MyBufferMutex;
	MyBufferMutex.Lock();
	m_PageOffset = position;
	if (lenght != ByteCapacity())
	{
		//last line could be NULL;
	}

	Clear(false, cursor_reset);
	wxString text_string;

	// Optimized Code
	//	for( unsigned i=0 ; i<lenght ; i++ )
		//text_string << text_ctrl->Filter(buffer[i]);
	//		text_string << static_cast<wxChar>((unsigned char)(buffer[i]));
	//		text_string << CP473toUnicode((unsigned char)(buffer[i]));

		//Painting Zebra Stripes, -1 means no stripe. 0 means start with normal, 1 means start with zebra
	*m_ZebraStriping = (m_ZebraEnable ? position / BytePerLine() % 2 : -1);

	if (m_SectorSize > 1)
	{
		m_OffsetControl->m_SectorSize = m_SectorSize;
		unsigned draw_line = m_SectorSize - (m_PageOffset % m_SectorSize);
		m_HexControl->m_ThinSeparationLines.Clear();
		m_TextControl->m_ThinSeparationLines.Clear();
		do
		{
			m_HexControl->m_ThinSeparationLines.Add(2 * draw_line);
			m_TextControl->m_ThinSeparationLines.Add(draw_line);
			draw_line += m_SectorSize;
		} while (draw_line < lenght);
	}
	else if (FakeBlockSize > 1)
	{
		m_OffsetControl->m_SectorSize = FakeBlockSize;
		unsigned draw_line = FakeBlockSize - (m_PageOffset % FakeBlockSize);
		m_HexControl->m_ThinSeparationLines.Clear();
		m_TextControl->m_ThinSeparationLines.Clear();
		do
		{
			m_HexControl->m_ThinSeparationLines.Add(2 * draw_line);
			m_TextControl->m_ThinSeparationLines.Add(draw_line);
			draw_line += FakeBlockSize;
		} while (draw_line < lenght);
	}
	else if (m_HexControl->m_ThinSeparationLines.Count())
	{
		m_HexControl->m_ThinSeparationLines.Clear();
		m_TextControl->m_ThinSeparationLines.Clear();
	}

	if (m_ProcessRAMMap.Count())
	{
		m_HexControl->m_ThinSeparationLines.Clear();
		m_TextControl->m_ThinSeparationLines.Clear();
		//Notice that, ProcessRAMMap is SORTED.
		for (unsigned i = 0; i < m_ProcessRAMMap.Count(); i++)
		{
			int64_t M = m_ProcessRAMMap.Item(i);
			if (M > m_PageOffset + ByteCapacity())
				break;

			if ((M > m_PageOffset)
				&& (M <= m_PageOffset + ByteCapacity()))
			{

				int draw_line = M - m_PageOffset;
				m_HexControl->m_ThinSeparationLines.Add(2 * draw_line);
				m_TextControl->m_ThinSeparationLines.Add(draw_line);
			}
		}
	}

	m_HexControl->SetBinValue(buffer, lenght, false);
	m_TextControl->SetBinValue(buffer, lenght, false);

	m_OffsetControl->SetValue(position, BytePerLine());

	if (m_OffsetScroll->GetThumbPosition() != (m_PageOffset / BytePerLine()))
		m_OffsetScroll->SetThumbPosition(m_PageOffset / BytePerLine());

	if (paint)
	{
		PaintSelection();
	}

	MyBufferMutex.Unlock();
}

wxHugeScrollBar::wxHugeScrollBar(wxScrollBar* m_scrollbar_)
{
	m_Range = m_Thumb = 0;
	m_ScrollBar = m_scrollbar_;

	m_ScrollBar->Connect(wxEVT_SCROLL_TOP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_BOTTOM, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Connect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
}

wxHugeScrollBar::~wxHugeScrollBar()
{
	m_ScrollBar->Disconnect(wxEVT_SCROLL_TOP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_BOTTOM, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
	m_ScrollBar->Disconnect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(wxHugeScrollBar::OnOffsetScroll), NULL, this);
}

void wxHugeScrollBar::SetThumbPosition(int64_t setpos)
{
#ifdef _DEBUG_SCROLL_
	std::cout << "SetThumbPosition()" << setpos << std::endl;
#endif
	m_Thumb = setpos;
	if (m_Range <= 2147483647)
	{
		m_ScrollBar->SetThumbPosition(setpos);
	}
	else
	{
#ifdef _DEBUG_SCROLL_
		std::cout << "m_Range: " << m_range << std::endl;
		std::cout << "SetThumbPositionx(): " << static_cast<int>(setpos * (2147483648.0 / m_range)) << std::endl;
#endif
		m_ScrollBar->SetThumbPosition(static_cast<int>(setpos * (2147483648.0 / m_Range)));
	}
}

void wxHugeScrollBar::SetScrollbar(int64_t Current_Position, int page_x, int64_t new_range, int pagesize, bool repaint)
{
	m_Range = new_range;
	if (new_range <= 2147483647)
	{ 
		//if representable with 32 bit
		m_ScrollBar->SetScrollbar(Current_Position, page_x, new_range, pagesize, repaint);
	}
	else {
#ifdef _DEBUG_SCROLL_
		std::cout << "new_range " << new_range << std::endl;
		std::cout << "Current_Position :" << (Current_Position * (2147483647 / new_range)) << std::endl;
#endif
		m_ScrollBar->SetScrollbar((Current_Position * (2147483647 / new_range)), page_x, 2147483647, pagesize, repaint);
	}
	SetThumbPosition(Current_Position);
}

void wxHugeScrollBar::OnOffsetScroll(wxScrollEvent& event)
{
	if (m_Range <= 2147483647)
	{
		m_Thumb = event.GetPosition();
	}
	else
	{
		//64bit mode
		int64_t here = event.GetPosition();
		if (here == 2147483646)	//if maximum set
			m_Thumb = m_Range - 1;	//than give maximum m_thumb which is -1 from range
		else
			m_Thumb = static_cast<int64_t>(here * (m_Range / 2147483647.0));
	}
	wxYieldIfNeeded();

#ifdef _DEBUG_SCROLL_
	if (event.GetEventType() == wxEVT_SCROLL_CHANGED)
		std::cout << "wxEVT_SCROLL_CHANGED" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
		std::cout << "wxEVT_SCROLL_THUMBTRACK" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
		std::cout << "wxEVT_SCROLL_THUMBRELEASE" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN)
		std::cout << "wxEVT_SCROLL_LINEDOWN" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_LINEUP)
		std::cout << "wxEVT_SCROLL_LINEUP" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_PAGEUP)
		std::cout << "wxEVT_SCROLL_PAGEUP" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)
		std::cout << "wxEVT_SCROLL_PAGEDOWN" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
		std::cout << "wxEVT_SCROLLWIN_LINEUP" << std::endl;
	if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
		std::cout << "wxEVT_SCROLLWIN_LINEDOWN" << std::endl;
#endif
	event.Skip();
}

void UDKHexEditor::PaintSelection()
{
	PreparePaintTAGs();
	if (m_Select->GetState())
	{
		int64_t start_byte = m_Select->m_StartOffset;
		int64_t end_byte = m_Select->m_EndOffset;

		if (start_byte > end_byte)
		{	// swap if start > end
			int64_t temp = start_byte;
			start_byte = end_byte;
			end_byte = temp;
		}

		if (start_byte >= m_PageOffset + GetByteCount())
		{
			// ...[..].TAG...
			ClearPaint();
			return;
		}
		else if (start_byte <= m_PageOffset)
		{
			// ...TA[G..]....
			start_byte = m_PageOffset;
		}

		if (end_byte < m_PageOffset)
		{
			// ..TAG..[...]...
			ClearPaint();
			return;
		}
		else if (end_byte >= m_PageOffset + GetByteCount())
		{
			//...[..T]AG...
			end_byte = GetByteCount() + m_PageOffset;
		}

		start_byte -= m_PageOffset;
		end_byte -= m_PageOffset;

		m_TextControl->SetSelection(start_byte / (GetCharToHexSize() / 2), end_byte / (GetCharToHexSize() / 2) + 1);
		m_HexControl->SetSelection(start_byte * 2, (end_byte + 1) * 2);
	}
	else
		ClearPaint();
}

void UDKHexEditor::PreparePaintTAGs()
{
	/*
	TagHideAll();
	WX_CLEAR_ARRAY(m_HexControl->m_TagArray);
	WX_CLEAR_ARRAY(m_TextControl->m_TagArray);

	if (!m_TagInvisible)
	{
		PaintTAGsPrefilter(MainTagArray);
	}

	PaintTAGsPrefilter(HighlightArray);

	PaintTAGsPrefilter(CompareArray);
	*/
}
