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

#include "LayerListBox.h"

#include "Model/Layer.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMouseEvent>

namespace TrenchBroom {
    namespace View {
        LayerListBoxWidget::LayerListBoxWidget(MapDocumentWPtr document, Model::Layer* layer, QWidget* parent) :
        ControlListBoxItemRenderer(parent),
        m_document(std::move(document)),
        m_layer(layer) {
            m_nameText = new QLabel(QString::fromStdString(m_layer->name()));
            // Ignore the label's minimum width, this prevents a horizontal scroll bar from appearing on the list widget,
            // and instead just cuts off the label for long layer names.
            m_nameText->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            m_infoText = new QLabel("");
            makeInfo(m_infoText);

            m_hiddenButton = createBitmapToggleButton("Hidden.png", "");
            m_lockButton = createBitmapToggleButton("Lock.png", "");

            MapDocumentSPtr documentS = lock(m_document);
            connect(m_hiddenButton, &QAbstractButton::clicked, this, [this](){
                emit layerVisibilityToggled(m_layer);
            });
            connect(m_lockButton, &QAbstractButton::clicked, this, [this]() {
                emit layerLockToggled(m_layer);
            });

            installEventFilter(this);
            m_nameText->installEventFilter(this);
            m_infoText->installEventFilter(this);

            auto* itemPanelBottomLayout = new QHBoxLayout();
            itemPanelBottomLayout->setContentsMargins(0, 0, 0, 0);
            itemPanelBottomLayout->setSpacing(0);

            itemPanelBottomLayout->addWidget(m_hiddenButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_lockButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_infoText, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addStretch(1);
            itemPanelBottomLayout->addSpacing(LayoutConstants::NarrowHMargin);

            auto* itemPanelLayout = new QVBoxLayout();
            itemPanelLayout->setContentsMargins(0, 0, 0, 0);
            itemPanelLayout->setSpacing(0);

            itemPanelLayout->addSpacing(LayoutConstants::NarrowVMargin);
            itemPanelLayout->addWidget(m_nameText);
            itemPanelLayout->addLayout(itemPanelBottomLayout);
            itemPanelLayout->addSpacing(LayoutConstants::NarrowVMargin);
            setLayout(itemPanelLayout);

            updateLayerItem();
        }

        void LayerListBoxWidget::updateItem() {
            updateLayerItem();
        }

        /**
         * This is factored out from updateItem() so the constructor can call it without doing a virtual function call
         */
        void LayerListBoxWidget::updateLayerItem() {
            // Update labels
            m_nameText->setText(QString::fromStdString(m_layer->name()));
            if (lock(m_document)->currentLayer() == m_layer) {
                makeEmphasized(m_nameText);
            } else {
                makeUnemphasized(m_nameText);
            }

            const auto info = tr("%1 %2").arg(m_layer->childCount()).arg(QString::fromStdString(StringUtils::safePlural(m_layer->childCount(), "object", "objects")));
            m_infoText->setText(info);

            // Update buttons
            m_lockButton->setChecked(m_layer->locked());
            m_hiddenButton->setChecked(m_layer->hidden());

            MapDocumentSPtr document = lock(m_document);
            m_lockButton->setEnabled(m_layer->locked() || m_layer != document->currentLayer());
            m_hiddenButton->setEnabled(m_layer->hidden() || m_layer != document->currentLayer());
        }

        Model::Layer* LayerListBoxWidget::layer() const {
            return m_layer;
        }

        bool LayerListBoxWidget::eventFilter(QObject* target, QEvent* event) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                auto* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    emit layerDoubleClicked(m_layer);
                    return true;
                }
            } else if (event->type() == QEvent::MouseButtonRelease) {
                auto* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::RightButton) {
                    emit layerRightClicked(m_layer);
                    return true;
                }
            }
            return QObject::eventFilter(target, event);
        }

        // LayerListBox

        LayerListBox::LayerListBox(MapDocumentWPtr document, QWidget* parent) :
        ControlListBox("", parent),
        m_document(std::move(document)) {
            bindObservers();
        }

        LayerListBox::~LayerListBox() {
            unbindObservers();
        }

        Model::Layer* LayerListBox::selectedLayer() const {
            return layerForRow(currentRow());
        }

        void LayerListBox::setSelectedLayer(Model::Layer* layer) {
            for (int i = 0; i < count(); ++i) {
                if (layerForRow(i) == layer) {
                    setCurrentRow(i);
                    break;
                }
            }
            setCurrentRow(-1);
        }

        void LayerListBox::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasClearedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->currentLayerDidChangeNotifier.addObserver(this, &LayerListBox::currentLayerDidChange);
            document->nodesWereAddedNotifier.addObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
            document->nodesWereRemovedNotifier.addObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
        }

        void LayerListBox::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasClearedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->currentLayerDidChangeNotifier.removeObserver(this, &LayerListBox::currentLayerDidChange);
                document->nodesWereAddedNotifier.removeObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
                document->nodesWereRemovedNotifier.removeObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
            }
        }

        void LayerListBox::documentDidChange(MapDocument* document) {
            reload();
        }

        void LayerListBox::nodesWereAddedOrRemoved(const Model::NodeList& nodes) {
            for (const auto* node : nodes) {
                if (node->depth() == 1) {
                    reload();
                    break;
                }
            }
            updateItems();
        }

        void LayerListBox::nodesDidChange(const Model::NodeList& nodes) {
            updateItems();
        }

        void LayerListBox::currentLayerDidChange(const Model::Layer* layer) {
            updateItems();
        }

        size_t LayerListBox::itemCount() const {
            MapDocumentSPtr document = lock(m_document);
            const auto* world = document->world();
            if (world == nullptr) {
                return 0;
            }
            return world->allLayers().size();
        }

        ControlListBoxItemRenderer* LayerListBox::createItemRenderer(QWidget* parent, const size_t index) {
            MapDocumentSPtr document = lock(m_document);
            const auto* world = document->world();
            const auto layers = world->allLayers();
            auto* renderer = new LayerListBoxWidget(document, layers[index], this);

            connect(renderer, &LayerListBoxWidget::layerDoubleClicked, this, [this](auto* layer){
                emit layerSetCurrent(layer);
            });
            connect(renderer, &LayerListBoxWidget::layerRightClicked, this, [this](auto* layer) {
                emit layerRightClicked(layer);
            });
            connect(renderer, &LayerListBoxWidget::layerVisibilityToggled, this, [this](auto* layer) {
                emit layerVisibilityToggled(layer);
            });
            connect(renderer, &LayerListBoxWidget::layerLockToggled, this, [this](auto* layer) {
                emit layerLockToggled(layer);
            });

            return renderer;
        }

        void LayerListBox::selectedRowChanged(const int index) {
            emit layerSelected(layerForRow(index));
        }

        const LayerListBoxWidget* LayerListBox::widgetAtRow(const int row) const {
            auto* renderer = this->renderer(row);
            if (renderer == nullptr) {
                return nullptr;
            } else {
                return static_cast<const LayerListBoxWidget*>(renderer);
            }
        }

        Model::Layer* LayerListBox::layerForRow(const int row) const {
            const auto* widget = widgetAtRow(row);
            if (widget == nullptr) {
                return nullptr;
            } else {
                return widget->layer();
            }
        }
    }
}
