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

#ifndef __TrenchBroom__DocManager__
#define __TrenchBroom__DocManager__

#include <wx/wx.h>
#include <wx/docview.h>
#include <wx/event.h>

class DocManager : public wxDocManager {
private:
    DECLARE_DYNAMIC_CLASS(DocManager)
    bool m_useSDI;
public:
    DocManager(long flags = wxDEFAULT_DOCMAN_FLAGS, bool initialize = true) : wxDocManager(flags, initialize), m_useSDI(false) {}
    
    inline bool GetUseSDI() const {
        return m_useSDI;
    }
    
    inline void SetUseSDI(bool useSDI) {
        m_useSDI = useSDI;
    }
    
    wxDocument* CreateDocument(const wxString& pathOrig, long flags = 0);
};

#endif /* defined(__TrenchBroom__DocManager__) */
