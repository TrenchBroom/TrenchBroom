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

#ifndef TrenchBroom_TitleBar
#define TrenchBroom_TitleBar

#include <QWidget>

class QLabel;

namespace TrenchBroom {
    namespace View {
        class TitleBar : public QWidget {
        protected:
            QLabel* m_titleText;
        public:
            TitleBar(const QString& title, QWidget* parent, int hMargin = 0, int vMargin = 0, bool boldTitle = true);
            explicit TitleBar(const QString& title, int hMargin = 0, int vMargin = 0, bool boldTitle = true);
        };
    }
}

#endif /* defined(TrenchBroom_TitleBar) */
