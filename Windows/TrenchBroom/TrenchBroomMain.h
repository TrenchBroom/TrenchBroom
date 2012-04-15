/***************************************************************
 * Name:      TrenchBroomMain.h
 * Purpose:   Defines Application Frame
 * Author:    Kristian Duske (kristian.duske@gmail.com)
 * Created:   2012-04-15
 * Copyright: Kristian Duske (kristianduske.com/trenchbroom)
 * License:
 **************************************************************/

#ifndef TRENCHBROOMMAIN_H
#define TRENCHBROOMMAIN_H

//(*Headers(TrenchBroomFrame)
#include <wx/menu.h>
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
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(TrenchBroomFrame)
        wxStatusBar* StatusBar1;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // TRENCHBROOMMAIN_H
