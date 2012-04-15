/*
 Copyright (C) 2010-2012 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wx_pch.h"
#include "TrenchBroomMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(TrenchBroomFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(TrenchBroomFrame)
const long TrenchBroomFrame::ID_DOCUMENTCANVAS = wxNewId();
const long TrenchBroomFrame::idMenuQuit = wxNewId();
const long TrenchBroomFrame::idMenuAbout = wxNewId();
const long TrenchBroomFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TrenchBroomFrame,wxFrame)
    //(*EventTable(TrenchBroomFrame)
    //*)
END_EVENT_TABLE()

TrenchBroomFrame::TrenchBroomFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(TrenchBroomFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, id, _("Map Document"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    SetClientSize(wxSize(800,600));
    int GLCanvasAttributes_1[] = {
    	WX_GL_RGBA,
    	WX_GL_DOUBLEBUFFER,
    	WX_GL_DEPTH_SIZE,      16,
    	WX_GL_STENCIL_SIZE,    8,
    	0, 0 };
    documentCanvas = new DocumentCanvas(this, ID_DOCUMENTCANVAS, wxPoint(232,432), wxDefaultSize, 0, _T("ID_DOCUMENTCANVAS"), GLCanvasAttributes_1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    Center();

    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&TrenchBroomFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&TrenchBroomFrame::OnAbout);
    //*)
}

TrenchBroomFrame::~TrenchBroomFrame()
{
    //(*Destroy(TrenchBroomFrame)
    //*)
}

void TrenchBroomFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void TrenchBroomFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}
