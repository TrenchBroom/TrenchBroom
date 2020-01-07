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

#include "KeySequenceEdit.h"

#include "View/LimitedKeySequenceEdit.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QBoxLayout>
#include <QAbstractButton>
#include <QStyle>

namespace TrenchBroom {
    namespace View {
        KeySequenceEdit::KeySequenceEdit(QWidget* parent) :
        KeySequenceEdit(LimitedKeySequenceEdit::MaxCount, parent) {}

        KeySequenceEdit::KeySequenceEdit(const size_t maxCount, QWidget* parent) :
        QWidget(parent),
        m_keySequenceEdit(nullptr),
        m_clearButton(nullptr){
            m_keySequenceEdit = new LimitedKeySequenceEdit(maxCount);
            m_keySequenceEdit->setToolTip("Click to start editing, then press the shortcut keys");
            m_clearButton = createBitmapButton(style()->standardIcon(QStyle::SP_LineEditClearButton), "Clear shortcut");

            setFocusProxy(m_keySequenceEdit);

            connect(m_keySequenceEdit, &LimitedKeySequenceEdit::editingFinished, this, &KeySequenceEdit::editingFinished);
            connect(m_keySequenceEdit, &LimitedKeySequenceEdit::keySequenceChanged, this, &KeySequenceEdit::keySequenceChanged);
            connect(m_clearButton, &QAbstractButton::clicked, this, &KeySequenceEdit::clear);

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(LayoutConstants::NarrowHMargin);
            layout->addWidget(m_keySequenceEdit, 1);
            layout->addWidget(m_clearButton);
            setLayout(layout);
        }

        const QKeySequence KeySequenceEdit::keySequence() const {
            return m_keySequenceEdit->keySequence();
        }

        void KeySequenceEdit::setKeySequence(const QKeySequence& keySequence) {
            m_keySequenceEdit->setKeySequence(keySequence);
        }

        void KeySequenceEdit::clear() {
            m_keySequenceEdit->clear();
        }
    }
}
