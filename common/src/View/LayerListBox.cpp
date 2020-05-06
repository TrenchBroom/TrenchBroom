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
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QLabel>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QListWidgetItem>
#include <QMouseEvent>

namespace TrenchBroom {
    namespace View {
        LayerListBoxWidget::LayerListBoxWidget(std::weak_ptr<MapDocument> document, Model::Layer* layer, QWidget* parent) :
        ControlListBoxItemRenderer(parent),
        m_document(std::move(document)),
        m_layer(layer) {
            m_nameText = new QLabel(QString::fromStdString(m_layer->name()));
            // Ignore the label's minimum width, this prevents a horizontal scroll bar from appearing on the list widget,
            // and instead just cuts off the label for long layer names.
            m_nameText->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            m_infoText = new QLabel("");
            makeInfo(m_infoText);

            m_hiddenButton = createBitmapToggleButton("Hidden.png", tr("Toggle hidden state"));
            m_lockButton = createBitmapToggleButton("Lock.png", tr("Toggle locked state"));
            m_moveLayerUpButton = createBitmapButton("Up.png", tr("Move the selected layer up"));
            m_moveLayerDownButton = createBitmapButton("Down.png", tr("Move the selected layer down"));

            auto documentS = kdl::mem_lock(m_document);
            connect(m_hiddenButton, &QAbstractButton::clicked, this, [this](){
                emit layerVisibilityToggled(m_layer);
            });
            connect(m_lockButton, &QAbstractButton::clicked, this, [this]() {
                emit layerLockToggled(m_layer);
            });
            connect(m_moveLayerUpButton, &QAbstractButton::clicked, this, [this]() {
                emit layerMovedUp(m_layer);
            });
            connect(m_moveLayerDownButton, &QAbstractButton::clicked, this, [this]() {
                emit layerMovedDown(m_layer);
            });

            installEventFilter(this);
            m_nameText->installEventFilter(this);
            m_infoText->installEventFilter(this);

            auto* textLayout = new QVBoxLayout();
            textLayout->setContentsMargins(0, LayoutConstants::NarrowVMargin, 0, LayoutConstants::NarrowVMargin);
            textLayout->setSpacing(LayoutConstants::NarrowVMargin);
            textLayout->addWidget(m_nameText, 1);
            textLayout->addWidget(m_infoText, 1);
            
            auto* itemPanelBottomLayout = new QHBoxLayout();
            itemPanelBottomLayout->setContentsMargins(0, 0, 0, 0);
            itemPanelBottomLayout->setSpacing(0);

            itemPanelBottomLayout->addWidget(m_hiddenButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_lockButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_moveLayerUpButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_moveLayerDownButton, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addWidget(m_infoText, 0, Qt::AlignVCenter);
            itemPanelBottomLayout->addStretch(1);
            itemPanelBottomLayout->addSpacing(LayoutConstants::NarrowHMargin);

            auto* itemPanelLayout = new QHBoxLayout();
            itemPanelLayout->setContentsMargins(0, 0, 0, 0);
            itemPanelLayout->setSpacing(LayoutConstants::MediumHMargin);

            itemPanelLayout->addLayout(textLayout, 1);
            itemPanelLayout->addWidget(m_hiddenButton);
            itemPanelLayout->addWidget(m_lockButton);
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
            if (kdl::mem_lock(m_document)->currentLayer() == m_layer) {
                makeEmphasized(m_nameText);
            } else {
                makeUnemphasized(m_nameText);
            }

            const auto info = tr("%1 %2").arg(m_layer->childCount()).arg(m_layer->childCount() == 1 ? "object" : "objects");
            m_infoText->setText(info);

            // Update buttons
            m_lockButton->setChecked(m_layer->locked());
            m_hiddenButton->setChecked(m_layer->hidden());

            auto document = lock(m_document);
            m_lockButton->setEnabled(m_layer->locked() || m_layer != document->currentLayer());
            m_hiddenButton->setEnabled(m_layer->hidden() || m_layer != document->currentLayer());

            const auto* world = document->world();
            m_moveLayerUpButton->setEnabled(m_layer != world->allLayers().front());
            m_moveLayerDownButton->setEnabled(m_layer != world->allLayers().back());
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

        LayerListBox::LayerListBox(std::weak_ptr<MapDocument> document, QWidget* parent) :
        ControlListBox("", true, parent),
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
            auto document = kdl::mem_lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasClearedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->currentLayerDidChangeNotifier.addObserver(this, &LayerListBox::currentLayerDidChange);
            document->nodesWereAddedNotifier.addObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
            document->nodesWereRemovedNotifier.addObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodeLockingDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
        }

        void LayerListBox::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasClearedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->currentLayerDidChangeNotifier.removeObserver(this, &LayerListBox::currentLayerDidChange);
                document->nodesWereAddedNotifier.removeObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
                document->nodesWereRemovedNotifier.removeObserver(this, &LayerListBox::nodesWereAddedOrRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
            }
        }

        void LayerListBox::documentDidChange(MapDocument*) {
            reload();
        }

        void LayerListBox::nodesWereAddedOrRemoved(const std::vector<Model::Node*>& nodes) {
            for (const auto* node : nodes) {
                if (dynamic_cast<const Model::Layer*>(node) != nullptr) {
                    // A layer was added or removed, so we need to clear and repopulate the list
                    reload();
                    return;
                }
            }
            updateItems();
        }

        void LayerListBox::nodesDidChange(const std::vector<Model::Node*>&) {
            updateItems();
        }

        void LayerListBox::currentLayerDidChange(const Model::Layer*) {
            updateItems();
        }

        size_t LayerListBox::itemCount() const {
            auto document = kdl::mem_lock(m_document);
            const auto* world = document->world();
            if (world == nullptr) {
                return 0;
            }
            return world->allLayers().size();
        }

        ControlListBoxItemRenderer* LayerListBox::createItemRenderer(QWidget* parent, const size_t index) {
            auto document = kdl::mem_lock(m_document);
            const auto* world = document->world();
            const auto layers = world->allLayers();
            auto* renderer = new LayerListBoxWidget(document, layers[index], parent);

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
            connect(renderer, &LayerListBoxWidget::layerMovedUp, this, [this](auto* layer) {
                emit layerMovedUp(layer);
            });
            connect(renderer, &LayerListBoxWidget::layerMovedDown, this, [this](auto* layer) {
                emit layerMovedDown(layer);
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
