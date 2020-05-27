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

#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QLabel>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QRadioButton>
#include <QToolButton>
#include <QtGlobal>

namespace TrenchBroom {
    namespace View {
        // LayerListBoxWidget

        LayerListBoxWidget::LayerListBoxWidget(std::weak_ptr<MapDocument> document, Model::LayerNode* layer, QWidget* parent) :
        ControlListBoxItemRenderer(parent),
        m_document(std::move(document)),
        m_layer(layer),
        m_activeButton(nullptr),
        m_nameText(nullptr),
        m_infoText(nullptr),
        m_hiddenButton(nullptr),
        m_lockButton(nullptr),
        m_moveLayerUpButton(nullptr),
        m_moveLayerDownButton(nullptr) {
            m_nameText = new QLabel(QString::fromStdString(m_layer->name()));
            // Ignore the label's minimum width, this prevents a horizontal scroll bar from appearing on the list widget,
            // and instead just cuts off the label for long layer names.
            m_nameText->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            m_infoText = new QLabel("");
            makeInfo(m_infoText);

            m_activeButton = new QRadioButton();
            m_hiddenButton = createBitmapToggleButton("Hidden.svg", tr("Toggle hidden state"));
            m_lockButton = createBitmapToggleButton("Lock.svg", tr("Toggle locked state"));
            m_moveLayerUpButton = createBitmapButton("Up.svg", tr("Move the selected layer up"));
            m_moveLayerDownButton = createBitmapButton("Down.svg", tr("Move the selected layer down"));

            auto documentS = kdl::mem_lock(m_document);
            connect(m_activeButton, &QAbstractButton::clicked, this, [this]() {
                emit layerActiveClicked(m_layer);
            });
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

            auto* itemPanelLayout = new QHBoxLayout();
            itemPanelLayout->setContentsMargins(0, 0, 0, 0);
            itemPanelLayout->setSpacing(LayoutConstants::MediumHMargin);

            itemPanelLayout->addWidget(m_activeButton);
            itemPanelLayout->addLayout(textLayout, 1);
            itemPanelLayout->addWidget(m_moveLayerUpButton, 0, Qt::AlignVCenter);
            itemPanelLayout->addWidget(m_moveLayerDownButton, 0, Qt::AlignVCenter);
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
            m_nameText->setText(tr("%1")
                .arg(QString::fromStdString(m_layer->name())));
            if (kdl::mem_lock(m_document)->currentLayer() == m_layer) {
                makeEmphasized(m_nameText);
            } else {
                makeUnemphasized(m_nameText);
            }

            const auto info = tr("%1 %2").arg(m_layer->childCount()).arg(m_layer->childCount() == 1 ? "object" : "objects");
            m_infoText->setText(info);

            // Update buttons
            auto document = kdl::mem_lock(m_document);
            m_activeButton->setChecked(document->currentLayer() == m_layer);
            m_lockButton->setChecked(m_layer->locked());
            m_hiddenButton->setChecked(m_layer->hidden());

            const auto* world = document->world();
            // defaultLayer can never move so just hide the buttons
            m_moveLayerUpButton->setVisible(m_layer != world->defaultLayer());
            m_moveLayerDownButton->setVisible(m_layer != world->defaultLayer());
            m_moveLayerUpButton->setEnabled(document->canMoveLayer(m_layer, -1));
            m_moveLayerDownButton->setEnabled(document->canMoveLayer(m_layer, 1));
        }

        Model::LayerNode* LayerListBoxWidget::layer() const {
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

        Model::LayerNode* LayerListBox::selectedLayer() const {
            return layerForRow(currentRow());
        }

        void LayerListBox::setSelectedLayer(Model::LayerNode* layer) {
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
            document->nodesWereAddedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
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
                document->nodesWereAddedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
            }
        }

        void LayerListBox::documentDidChange(MapDocument*) {
            reload();
        }

        void LayerListBox::nodesDidChange(const std::vector<Model::Node*>& nodes) {
            for (const auto* node : nodes) {
                if (dynamic_cast<const Model::LayerNode*>(node) != nullptr) {
                    // A layer was added or removed or modified, so we need to clear and repopulate the list
                    reload();
                    return;
                }
            }
            updateItems();
        }

        void LayerListBox::currentLayerDidChange(const Model::LayerNode*) {
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

            Model::LayerNode* layer;
            if (index == 0) {
                layer = world->defaultLayer();
            } else {
                layer = world->customLayersUserSorted().at(index - 1);
            }
            auto* renderer = new LayerListBoxWidget(document, layer, parent);
            connect(renderer, &LayerListBoxWidget::layerActiveClicked,     this, &LayerListBox::layerSetCurrent);
            connect(renderer, &LayerListBoxWidget::layerDoubleClicked,     this, &LayerListBox::layerSetCurrent);
            connect(renderer, &LayerListBoxWidget::layerRightClicked,      this, &LayerListBox::layerRightClicked);
            connect(renderer, &LayerListBoxWidget::layerVisibilityToggled, this, &LayerListBox::layerVisibilityToggled);
            connect(renderer, &LayerListBoxWidget::layerLockToggled,       this, &LayerListBox::layerLockToggled);
            connect(renderer, &LayerListBoxWidget::layerMovedUp,           this, &LayerListBox::layerMovedUp);
            connect(renderer, &LayerListBoxWidget::layerMovedDown,         this, &LayerListBox::layerMovedDown);
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

        Model::LayerNode* LayerListBox::layerForRow(const int row) const {
            const auto* widget = widgetAtRow(row);
            if (widget == nullptr) {
                return nullptr;
            } else {
                return widget->layer();
            }
        }
    }
}
