/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "PersistentSplitterWindow2.h"

#include "View/SplitterWindow2.h"

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        const double PersistentSplitterWindow2::Scaling = 10000.0;
        
        PersistentSplitterWindow2::PersistentSplitterWindow2(SplitterWindow2* obj) :
        wxPersistentWindow(obj) {}
        
        wxString PersistentSplitterWindow2::GetKind() const {
            return "SplitterWindow2";
        }
        
        void PersistentSplitterWindow2::Save() const {
            const SplitterWindow2* window = Get();
            const double ratio = window->m_currentSplitRatio == -1.0 ? window->m_initialSplitRatio : window->m_currentSplitRatio;
            const wxCoord scaledRatio = static_cast<int>(Scaling * ratio);
            SaveValue("SplitRatio", scaledRatio);
        }
        
        bool PersistentSplitterWindow2::Restore() {
            int scaledRatio = -1;
            if (!RestoreValue("SplitRatio", &scaledRatio))
                return false;
            
            SplitterWindow2* window = Get();
            window->m_initialSplitRatio = std::max(-1.0, std::min(1.0, static_cast<double>(scaledRatio) / Scaling));
            return true;
        }
    }
}
