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

#ifndef TrenchBroom_RecentDocumentListBox
#define TrenchBroom_RecentDocumentListBox

#include "View/ImageListBox.h"

#include <wx/bitmap.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class RecentDocumentListBox : public ImageListBox {
        private:
            wxBitmap m_documentIcon;
        public:
            RecentDocumentListBox(wxWindow* parent);
            ~RecentDocumentListBox();
            
            void OnListBoxDoubleClick(wxCommandEvent& event);
        private:
            void recentDocumentsDidChange();
            
            bool image(size_t n, wxBitmap& result) const;
            wxString title(size_t n) const;
            wxString subtitle(size_t n) const;
        };
    }
}


#endif /* defined(TrenchBroom_RecentDocumentListBox) */
