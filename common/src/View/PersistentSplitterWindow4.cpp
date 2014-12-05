/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PersistentSplitterWindow4.h"

#include "View/SplitterWindow4.h"

namespace TrenchBroom {
    namespace View {
        PersistentSplitterWindow4::PersistentSplitterWindow4(SplitterWindow4* obj) :
        wxPersistentWindow(obj) {}
        
        wxString PersistentSplitterWindow4::GetKind() const {
            return "SplitterWindow4";
        }
        
        void PersistentSplitterWindow4::Save() const {
            const SplitterWindow4* window = Get();
            SaveValue("SashPositionX", window->m_sashPosition.x);
            SaveValue("SashPositionY", window->m_sashPosition.y);
        }
        
        bool PersistentSplitterWindow4::Restore() {
            wxPoint sashPosition(-1, -1);
            if (!RestoreValue("SashPositionX", &sashPosition.x))
                return false;
            if (!RestoreValue("SashPositionY", &sashPosition.y))
                return false;
            
            SplitterWindow4* window = Get();
            window->m_initialSashPosition = sashPosition;
            return true;
        }
    }
}
