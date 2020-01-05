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

#include "View/BorderLine.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        // ControlListBoxItemRenderer

        ControlListBoxItemRenderer::ControlListBoxItemRenderer(QWidget* parent) :
        QWidget(parent),
        m_index(0) {}

        ControlListBoxItemRenderer::~ControlListBoxItemRenderer() = default;

        void ControlListBoxItemRenderer::setIndex(const size_t index) {
            m_index = index;
        }

        void ControlListBoxItemRenderer::mouseDoubleClickEvent(QMouseEvent* event) {
            QWidget::mouseDoubleClickEvent(event);
            if (event->button() == Qt::LeftButton) {
                emit doubleClicked(m_index);
            }
        }

        void ControlListBoxItemRenderer::updateItem() {}

        void ControlListBoxItemRenderer::setSelected(const bool selected) {
            // by default, we just change the appearance of all labels
            auto children = findChildren<QLabel*>();
            for (auto* child : children) {
                const auto dontUpdate = child->property(ControlListBox::LabelColorShouldNotUpdateWhenSelected);
                if (dontUpdate.isValid() && dontUpdate.canConvert(QMetaType::Bool) && dontUpdate.toBool()) {
                    continue;
                }
                if (selected) {
                    makeSelected(child);
                } else {
                    makeUnselected(child);
                }
            }
        }

        // ControlListBoxItemRendererWrapper

        ControlListBoxItemRendererWrapper::ControlListBoxItemRendererWrapper(ControlListBoxItemRenderer* renderer, const bool showSeparator, QWidget* parent) :
        QWidget(parent),
        m_renderer(renderer) {
            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_renderer);

            if (showSeparator) {
                layout->addWidget(new BorderLine());
            }

            setLayout(layout);
        }

        ControlListBoxItemRenderer* ControlListBoxItemRendererWrapper::renderer() {
            return m_renderer;
        }

        const ControlListBoxItemRenderer* ControlListBoxItemRendererWrapper::renderer() const {
            return m_renderer;
        }

        // ControlListBox

        ControlListBox::ControlListBox(const QString& emptyText, const QMargins& itemMargins, const bool showSeparator, QWidget* parent) :
        QWidget(parent),
        m_listWidget(new QListWidget()),
        m_emptyTextContainer(new QWidget()),
        m_emptyTextLabel(new QLabel(emptyText)),
        m_itemMargins(itemMargins),
        m_showSeparator(showSeparator) {
            m_listWidget->setObjectName("controlListBox_listWidget");
            m_listWidget->hide();
            m_listWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
            // m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &ControlListBox::listItemSelectionChanged);

            m_emptyTextLabel->setWordWrap(true);
            m_emptyTextLabel->setDisabled(true);
            m_emptyTextLabel->setAlignment(Qt::AlignHCenter);
            m_emptyTextLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            setLayout(outerLayout);

            outerLayout->addWidget(m_listWidget, 1);
            outerLayout->addWidget(m_emptyTextContainer);

            auto* emptyTextLayout = new QVBoxLayout();
            m_emptyTextContainer->setLayout(emptyTextLayout);
            emptyTextLayout->addWidget(m_emptyTextLabel);
        }

        ControlListBox::ControlListBox(const QString& emptyText, const bool showSeparator, QWidget* parent) :
        ControlListBox(emptyText, QMargins(LayoutConstants::MediumHMargin, LayoutConstants::NarrowVMargin, LayoutConstants::MediumHMargin, LayoutConstants::NarrowVMargin), showSeparator, parent) {}

        void ControlListBox::setEmptyText(const QString& emptyText) {
            m_emptyTextLabel->setText(emptyText);
        }

        void ControlListBox::setItemMargins(const QMargins& itemMargins) {
            m_itemMargins = itemMargins;
            reload();
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

        void ControlListBox::reload() {
            DisableWindowUpdates disableUpdates(this);

            // WARNING: At this point, the ControlListBoxItemRenderer's might
            // contain dangling pointers to model objects (if
            // MapDocument::clearWorld world is called, e.g. when opening a new map).
            //
            // The clear() call below causes QListWidget::itemSelectionChanged
            // to be emitted, before the widgets are cleared.
            // This was causing a crash in LayerListBox's selectedRowChanged() override
            // if you clicked on a layer and then opened a new map on Windows.
            // As a workaround, unset the current row before clearing the list.
            m_listWidget->setCurrentRow(-1);

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
        }

        void ControlListBox::updateItems() {
            DisableWindowUpdates disableUpdates(this);
            for (int i = 0; i < m_listWidget->count(); ++i) {
                auto* renderer = this->renderer(i);
                renderer->updateItem();
            }
        }

        const ControlListBoxItemRenderer* ControlListBox::renderer(const int i) const {
            auto* wrapper = this->wrapper(i);
            if (wrapper == nullptr) {
                return nullptr;
            }
            return wrapper->renderer();
        }

        ControlListBoxItemRenderer* ControlListBox::renderer(const int i) {
            auto* wrapper = this->wrapper(i);
            if (wrapper == nullptr) {
                return nullptr;
            }
            return wrapper->renderer();
        }

        ControlListBoxItemRendererWrapper* ControlListBox::wrapper(int i) const {
            if (i < 0 || i >= count()) {
                return nullptr;
            }
            auto* widgetItem = m_listWidget->item(i);
            return static_cast<ControlListBoxItemRendererWrapper*>(m_listWidget->itemWidget(widgetItem));
        }

        ControlListBoxItemRendererWrapper* ControlListBox::wrapper(const int i) {
            if (i < 0 || i >= count()) {
                return nullptr;
            }
            auto* widgetItem = m_listWidget->item(i);
            return static_cast<ControlListBoxItemRendererWrapper*>(m_listWidget->itemWidget(widgetItem));
        }

        void ControlListBox::addItemRenderer(ControlListBoxItemRenderer* renderer) {
            const auto index = count();
            renderer->setIndex(static_cast<size_t>(index));
            renderer->setContentsMargins(m_itemMargins);
            connect(renderer, &ControlListBoxItemRenderer::doubleClicked, this, &ControlListBox::doubleClicked);

            auto* widgetItem = new QListWidgetItem(m_listWidget);
            m_listWidget->addItem(widgetItem);

            if (m_listWidget->itemWidget(widgetItem) != nullptr) {
                m_listWidget->removeItemWidget(widgetItem);
            }

            auto* wrapper = new ControlListBoxItemRendererWrapper(renderer, m_showSeparator);

            m_listWidget->setItemWidget(widgetItem, wrapper);
            widgetItem->setSizeHint(renderer->minimumSizeHint());
            renderer->updateItem();
            renderer->setSelected(m_listWidget->currentItem() == widgetItem);
        }

        void ControlListBox::selectedRowChanged(const int /* index */) {}

        void ControlListBox::doubleClicked(const size_t /* index */) {}

        void ControlListBox::listItemSelectionChanged() {
            for (int row = 0; row < count(); ++row) {
                auto* listItem = m_listWidget->item(row);
                auto* renderer = this->renderer(row);
                renderer->setSelected(listItem->isSelected());
                if (listItem->isSelected()) {
                    selectedRowChanged(row);
                }
            }

            emit itemSelectionChanged();
        }
    }
}
