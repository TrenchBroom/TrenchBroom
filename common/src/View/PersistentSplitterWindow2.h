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

#ifndef TrenchBroom_PersistentSplitterWindow2
#define TrenchBroom_PersistentSplitterWindow2

#include <wx/persist/toplevel.h>

namespace TrenchBroom {
    namespace View {
        class SplitterWindow2;
        
        class PersistentSplitterWindow2 : public wxPersistentWindow<SplitterWindow2> {
        public:
            PersistentSplitterWindow2(SplitterWindow2* obj);
            
            wxString GetKind() const;
            void Save() const;
            bool Restore();
        };
    }
}

#endif /* defined(TrenchBroom_PersistentSplitterWindow2) */
