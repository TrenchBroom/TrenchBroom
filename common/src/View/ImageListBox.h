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

#ifndef TrenchBroom_ImageListBox
#define TrenchBroom_ImageListBox

#include "ControlListBox.h"

class QLabel;
class wxStaticBitmap;

namespace TrenchBroom {
    namespace View {
        class ImageListBox : public ControlListBox {
        public:
            ImageListBox(QWidget* parent, const QString& emptyText);
        private:
            class ImageListBoxItem;
            Item* createItem(QWidget* parent, const wxSize& margins, size_t index) override;
        private:
            virtual bool image(size_t index, wxBitmap& result) const;
            virtual QString title(size_t index) const = 0;
            virtual QString subtitle(size_t index) const = 0;
        };
    }
}


#endif /* defined(TrenchBroom_ImageListBox) */
