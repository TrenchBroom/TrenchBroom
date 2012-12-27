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

#ifndef __TrenchBroom__SpinControl__
#define __TrenchBroom__SpinControl__

#include <wx/spinctrl.h>

class SpinControl : public wxSpinCtrlDouble {
public:
    SpinControl(wxWindow* parent, wxWindowID id) :
    wxSpinCtrlDouble(parent, id) {}
    
    void SetHint(const wxString& hint) {
#ifndef __linux__
        GetText()->SetHint(hint);
#endif
    }
};

#endif /* defined(__TrenchBroom__SpinControl__) */
