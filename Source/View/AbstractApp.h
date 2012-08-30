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

#ifndef __TrenchBroom__AbstractApp__
#define __TrenchBroom__AbstractApp__

#include <wx/wx.h>

class wxDocManager;
class wxCommandEvent;
class wxMenu;
class wxMenuBar;

class AbstractApp : public wxApp {
protected:
	wxDocManager* m_docManager;
    wxMenuBar* m_menuBar;
    
    virtual wxMenu* CreateFileMenu();
    virtual void PostInit() = 0;
public:
	virtual bool OnInit();
    virtual int OnExit();
    void OnUnhandledException();
    
    virtual void OnFileExit(wxCommandEvent& event);
    
    DECLARE_EVENT_TABLE();
};

#endif /* defined(__TrenchBroom__AbstractApp__) */
