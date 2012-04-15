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

#ifndef TRENCHBROOMMAIN_H
#define TRENCHBROOMMAIN_H

//(*Headers(TrenchBroomFrame)
#include <wx/menu.h>
#include "DocumentCanvas.h"
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

class TrenchBroomFrame: public wxFrame
{
    public:

        TrenchBroomFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~TrenchBroomFrame();

    private:

        //(*Handlers(TrenchBroomFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        //*)

        //(*Identifiers(TrenchBroomFrame)
        static const long ID_DOCUMENTCANVAS;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(TrenchBroomFrame)
        DocumentCanvas* documentCanvas;
        wxStatusBar* StatusBar1;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // TRENCHBROOMMAIN_H
