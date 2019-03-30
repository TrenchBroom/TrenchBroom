/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <wx/bitmap.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class GameListBox : public ImageListBox {
        private:
            struct Info {
                wxBitmap image;
                QString title;
                QString subtitle;
            };

            using InfoList = std::vector<Info>;

            InfoList m_gameInfos;
        public:
            GameListBox(QWidget* parent);

            String selectedGameName() const;
            void selectGame(int index);

            void OnListBoxChange();
            void OnListBoxDoubleClick();
            void reloadGameInfos();
        private:
            bool image(size_t n, wxBitmap& result) const override;
            QString title(size_t n) const override;
            QString subtitle(size_t n) const override;

            void submitChangeEvent(wxEventType type);
        };
    }
}


#endif /* defined(TrenchBroom_GameListBox) */
