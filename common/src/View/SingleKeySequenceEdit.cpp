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

#include "SingleKeySequenceEdit.h"

#include <QKeyEvent>

namespace TrenchBroom {
    namespace View {
        SingleKeySequenceEdit::SingleKeySequenceEdit(QWidget* parent) :
        QKeySequenceEdit(parent),
        m_count(0) {
            connect(this, &QKeySequenceEdit::editingFinished, this, &SingleKeySequenceEdit::resetCount);
        }

        void SingleKeySequenceEdit::keyPressEvent(QKeyEvent* event) {
            if (m_count == 0 && event->modifiers() == Qt::NoModifier) {
                QKeySequenceEdit::keyPressEvent(event);
                ++m_count;
            }
        }

        void SingleKeySequenceEdit::resetCount() {
            m_count = 0;
        }
    }
}
