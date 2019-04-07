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

#include <iostream>

namespace TrenchBroom {
    namespace View {
        ControlListBoxItemRenderer::ControlListBoxItemRenderer(QWidget* parent) :
        QWidget(parent) {}

        ControlListBoxItemRenderer::~ControlListBoxItemRenderer() = default;

        void ControlListBoxItemRenderer::update(const size_t index) {}

        void ControlListBoxItemRenderer::setSelected(const bool selected) {}

        ControlListBox::ControlListBox(const QString& emptyText, QWidget* parent) :
        QWidget(parent),
        m_listWidget(new QListWidget()),
        m_emptyTextContainer(new QWidget()),
        m_emptyTextLabel(new QLabel(emptyText)) {
            m_listWidget->setObjectName("controlListBox_listWidget");
            m_listWidget->hide();
            m_listWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            connect(m_listWidget, &QListWidget::currentItemChanged, this, &ControlListBox::currentItemChanged);
            connect(m_listWidget, &QListWidget::currentRowChanged, this, &ControlListBox::currentRowChanged);

            m_emptyTextLabel->setWordWrap(true);
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

            setStyleSheet("QListWidget#controlListBox_listWidget { border: none; }");
        }

        int ControlListBox::count() const {
            return m_listWidget->count();
        }

        int ControlListBox::currentRow() const {
            return m_listWidget->currentRow();
        }

        void ControlListBox::setCurrentRow(const int currentRow) {
            m_listWidget->setCurrentRow(currentRow);
        }

        void ControlListBox::setEmptyText(const QString& emptyText) {
            m_emptyTextLabel->setText(emptyText);
        }

        void ControlListBox::reload() {
            setUpdatesEnabled(false);

            m_listWidget->clear();

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

        void ControlListBox::updateItems() {
            setUpdatesEnabled(false);
            for (int i = 0; i < m_listWidget->count(); ++i) {
                auto* widgetItem = m_listWidget->item(i);
                auto* renderer = static_cast<ControlListBoxItemRenderer*>(m_listWidget->itemWidget(widgetItem));
                renderer->update(static_cast<size_t>(i));
            }
            setUpdatesEnabled(true);
        }

        void ControlListBox::addItemRenderer(ControlListBoxItemRenderer* renderer) {
            auto* widgetItem = new QListWidgetItem(m_listWidget);
            m_listWidget->addItem(widgetItem);
            setItemRenderer(widgetItem, renderer);
        }

        void ControlListBox::setItemRenderer(QListWidgetItem* widgetItem, ControlListBoxItemRenderer* renderer) {
            if (m_listWidget->itemWidget(widgetItem) != nullptr) {
                m_listWidget->removeItemWidget(widgetItem);
            }
            m_listWidget->setItemWidget(widgetItem, renderer);
            widgetItem->setSizeHint(renderer->minimumSizeHint());
            renderer->setSelected(m_listWidget->currentItem() == widgetItem);
        }

        void ControlListBox::currentRowChanged(const int index) {}

        void ControlListBox::currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous) {
            if (previous != nullptr) {
                auto* previousRenderer = static_cast<ControlListBoxItemRenderer*>(m_listWidget->itemWidget(previous));
                previousRenderer->setSelected(false);
            }
            if (current != nullptr) {
                auto* currentRenderer = static_cast<ControlListBoxItemRenderer*>(m_listWidget->itemWidget(current));
                currentRenderer->setSelected(true);
            }
        }
    }
}
