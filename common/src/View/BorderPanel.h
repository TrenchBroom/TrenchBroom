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

#pragma once

#include <QWidget>

namespace TrenchBroom {
    namespace View {
        class BorderPanel : public QWidget {
            Q_OBJECT
        public:
            enum Sides {
                TopSide = 1,
                RightSide = 2,
                BottomSide = 4,
                LeftSide = 8,
                AllSides = TopSide | RightSide | BottomSide | LeftSide
            };
        private:
            int m_borders;
            int m_thickness;
        public:
            explicit BorderPanel(Sides borders = AllSides, int thickness = 1, QWidget* parent = nullptr);
        protected:
            void paintEvent(QPaintEvent *event) override;
        };
    }
}

