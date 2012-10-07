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

#ifndef __TrenchBroom__MacScreenDC__
#define __TrenchBroom__MacScreenDC__

#include "wx/defs.h"
#include "wx/dc.h"

#include "wx/dcclient.h"
#include "wx/osx/dcclient.h"

class MacScreenDCImpl: public wxWindowDCImpl {
public:
    MacScreenDCImpl( wxDC *owner );
    virtual ~MacScreenDCImpl();
    
    virtual void Flush();
    virtual wxBitmap DoGetAsBitmap(const wxRect *subrect) const;
private:
    void* m_overlayWindow;
    void* m_nativeGraphicsContext;
    
private:
    DECLARE_CLASS(MacScreenDCImpl)
    wxDECLARE_NO_COPY_CLASS(MacScreenDCImpl);
};

class MacScreenDC : public wxDC {
public:
    MacScreenDC() : wxDC(new MacScreenDCImpl(this)) {}
    
    static bool StartDrawingOnTop(wxWindow * WXUNUSED(window)) {
        return true;
    }

    static bool StartDrawingOnTop(wxRect * WXUNUSED(rect) =  NULL) {
        return true;
    }
    
    static bool EndDrawingOnTop() {
        return true;
    }
    
    void Flush() {
        m_pimpl->Flush();
    }
};

#endif /* defined(__TrenchBroom__MacScreenDC__) */
