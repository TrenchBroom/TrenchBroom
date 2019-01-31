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

#include "PopupButton.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"
#include "View/PopupWindow.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QWindow>
#include <QScreen>

namespace TrenchBroom {
    namespace View {
        PopupButton::PopupButton(QWidget* parent, const QString& caption) :
        QWidget(parent) {
            m_button = new QToolButton();
            m_button->setText(caption);
            m_button->setCheckable(true);

            m_window = new PopupWindow(this);

            auto* sizer = new QHBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_button);
            setLayout(sizer);

            connect(m_button, &QAbstractButton::clicked, this, &PopupButton::OnButtonToggled);
            connect(m_window, &PopupWindow::visibilityChanged, this, &PopupButton::OnPopupVisibilityChanged);
        }

        QWidget* PopupButton::GetPopupWindow() const {
            return m_window;
        }

        void PopupButton::OnButtonToggled(bool checked) {
            if (checked) {
                m_window->show();
                m_window->positionTouchingWidget(this);
            } else {
                m_window->close();
            }
        }

        void PopupButton::OnPopupVisibilityChanged(bool visible) {
            m_button->setChecked(visible);
        }
    }
}
