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

#include "ControlListBox.h"

#include <QLabel>
#include <QListWidget>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        ControlListBox::ItemRenderer::ItemRenderer(QWidget* parent) :
        QWidget(parent) {}

        ControlListBox::ItemRenderer::~ItemRenderer() = default;

        void ControlListBox::ItemRenderer::setSelectionColors(const QColor& foreground, const QColor& background) {
            setColors(this, foreground, background);
        }

        void ControlListBox::ItemRenderer::setDefaultColors(const QColor& foreground, const QColor& background) {
            setColors(this, foreground, background);
        }

        void ControlListBox::ItemRenderer::setColors(QWidget* window, const QColor& foreground, const QColor& background) {
        }

        ControlListBox::ControlListBox(const QString& emptyText, QWidget* parent) :
        QWidget(parent),
        m_listWidget(new QListWidget()),
        m_emptyTextContainer(new QWidget()),
        m_emptyTextLabel(new QLabel(emptyText)) {
            m_listWidget->hide();
            m_listWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

            m_emptyTextLabel->setDisabled(true);
            m_emptyTextLabel->setAlignment(Qt::AlignHCenter);
            m_emptyTextLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            setLayout(outerLayout);

            outerLayout->addWidget(m_listWidget, 1);
            outerLayout->addWidget(m_emptyTextContainer);

            auto* emptyTextLayout = new QVBoxLayout();
            m_emptyTextContainer->setLayout(emptyTextLayout);
            emptyTextLayout->addWidget(m_emptyTextLabel);
        }

        void ControlListBox::setEmptyText(const QString& emptyText) {
            m_emptyTextLabel->setText(emptyText);
        }

        void ControlListBox::refresh() {
            setUpdatesEnabled(false);

            clear();

            const auto count = itemCount();
            if (count > 0) {
                for (size_t i = 0; i < count; ++i) {
                    addItemRenderer(createItemRenderer(m_listWidget, i));
                }
                m_listWidget->show();
                m_emptyTextContainer->hide();
            } else {
                m_listWidget->hide();
                m_emptyTextContainer->show();
            }

            setUpdatesEnabled(true);
        }

        void ControlListBox::clear() {
            m_listWidget->clear();
        }

        void ControlListBox::addItemRenderer(ItemRenderer* renderer) {
            auto* widgetItem = new QListWidgetItem(m_listWidget);
            m_listWidget->addItem(widgetItem);

            widgetItem->setSizeHint(renderer->minimumSizeHint());
            m_listWidget->setItemWidget(widgetItem, renderer);
        }
    }
}
