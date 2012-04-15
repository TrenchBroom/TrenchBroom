/***************************************************************
 * Name:      TrenchBroomApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Kristian Duske (kristian.duske@gmail.com)
 * Created:   2012-04-15
 * Copyright: Kristian Duske (kristianduske.com/trenchbroom)
 * License:
 **************************************************************/

#include "TrenchBroomApp.h"

//(*AppHeaders
#include "TrenchBroomMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(TrenchBroomApp);

bool TrenchBroomApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	TrenchBroomFrame* Frame = new TrenchBroomFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}
