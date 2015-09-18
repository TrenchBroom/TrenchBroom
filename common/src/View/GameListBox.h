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

#ifndef TrenchBroom_GameListBox
#define TrenchBroom_GameListBox

#include "StringUtils.h"
#include "View/ImageListBox.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class GameListBox : public ImageListBox {
        private:
            struct Info {
                wxBitmap image;
                wxString title;
                wxString subtitle;
            };
            
            typedef std::vector<Info> InfoList;
            
            InfoList m_gameInfos;
        public:
            GameListBox(wxWindow* parent, long style = wxBORDER_NONE);
            
            String selectedGameName() const;
            void selectGame(int index);
            
            void OnListBoxChange(wxCommandEvent& event);
            void OnListBoxDoubleClick(wxCommandEvent& event);
            void reloadGameInfos();
        private:
            const wxBitmap& image(const size_t n) const;
            wxString title(const size_t n) const;
            wxString subtitle(const size_t n) const;
            
            void submitChangeEvent(wxEventType type);
        };
    }
}


#endif /* defined(TrenchBroom_GameListBox) */
