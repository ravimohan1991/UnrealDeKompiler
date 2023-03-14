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

class UDKHexEditor : public UDKControl
{
public:
	UDKHexEditor(wxWindow* parent,
		int id,
		wxStatusBar* statusbar = NULL,
		DataInterpreter* interpreter = NULL,
		InfoPanel* infopanel = NULL,
		TagPanel* tagpanel = NULL,
		DisassemblerPanel* dasmpanel = NULL,
		copy_maker* copy_mark = NULL,
		wxFileName* myfile = NULL,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
};