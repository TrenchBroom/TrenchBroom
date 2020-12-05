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

#pragma once

#include <QSplitter>

namespace TrenchBroom {
    namespace View {
        class SplitterHandle : public QSplitterHandle {
            Q_OBJECT
        public:
            explicit SplitterHandle(Qt::Orientation orientation, QSplitter* parent = nullptr);

            QSize sizeHint() const override;
        protected:
            void paintEvent(QPaintEvent* event) override;
        };

        class Splitter : public QSplitter {
            Q_OBJECT
        public:
            explicit Splitter(Qt::Orientation orientation, QWidget *parent = nullptr);
            explicit Splitter(QWidget* parent = nullptr);
        protected:
            QSplitterHandle* createHandle() override;

#ifdef __APPLE__
        // on macOS, the widgets are not repainted properly when the splitter moves, so we force them to repaint
        private slots:
            void doSplitterMoved();
#endif
        };
    }
}


