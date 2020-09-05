/*
 Copyright (C) 2020 Kristian Duske

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

#ifndef TrenchBroom_ClickableLabel
#define TrenchBroom_ClickableLabel

#include <QLabel>

namespace TrenchBroom {
    namespace View {
        class ClickableLabel : public QLabel {
            Q_OBJECT
        public:
            explicit ClickableLabel(const QString& text, QWidget* parent = nullptr);

        protected:
            void mousePressEvent(QMouseEvent *event) override;

        signals:
            void clicked();
        };
    }
}

#endif /* defined(TrenchBroom_ClickableLabel) */
