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

namespace TrenchBroom {
    namespace View {
        // LayerListBoxLayerItem

        LayerListBoxLayerItem::LayerListBoxLayerItem(QWidget* parent, MapDocumentWPtr document, Model::Layer* layer) :
        QWidget(parent),
        m_document(document),
        m_layer(layer) {
            m_nameText = new QLabel(QString::fromStdString(m_layer->name()));
            // Ignore the label's minimum width, this prevents a horizontal scroll bar from appearing on the list widget,
            // and instead just cuts off the label for long layer names.
            m_nameText->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            m_infoText = new QLabel("");
            makeInfo(m_infoText);
            refresh();

            m_hiddenButton = createBitmapToggleButton(this, "Visible.png", "");
            m_lockButton = createBitmapToggleButton(this, "Lock.png", "");

            MapDocumentSPtr documentS = lock(m_document);
            m_hiddenButton->setEnabled(m_layer->hidden() || m_layer != documentS->currentLayer());
            m_lockButton->setEnabled(m_layer->locked() || m_layer != documentS->currentLayer());

            connect(m_hiddenButton, &QAbstractButton::toggled, this, &LayerListBoxLayerItem::OnToggleVisible);
            connect(m_lockButton, &QAbstractButton::toggled, this, &LayerListBoxLayerItem::OnToggleLocked);

            auto* itemPanelBottomSizer = new QHBoxLayout();
            itemPanelBottomSizer->setContentsMargins(0, 0, 0, 0);
            itemPanelBottomSizer->setSpacing(0);

            itemPanelBottomSizer->addWidget(m_hiddenButton, 0, Qt::AlignVCenter);
            itemPanelBottomSizer->addWidget(m_lockButton, 0, Qt::AlignVCenter);
            itemPanelBottomSizer->addWidget(m_infoText, 0, Qt::AlignVCenter);
            itemPanelBottomSizer->addStretch(1);
            itemPanelBottomSizer->addSpacing(LayoutConstants::NarrowHMargin);

            auto* itemPanelSizer = new QVBoxLayout();
            itemPanelSizer->setContentsMargins(0, 0, 0, 0);
            itemPanelSizer->setSpacing(0);

            itemPanelSizer->addSpacing(LayoutConstants::NarrowVMargin);
            itemPanelSizer->addWidget(m_nameText);
            itemPanelSizer->addLayout(itemPanelBottomSizer);
            itemPanelSizer->addSpacing(LayoutConstants::NarrowVMargin);
            setLayout(itemPanelSizer);

            updateButtons();

            // FIXME: Listen for layer lock/visible changes
        }

        Model::Layer* LayerListBoxLayerItem::layer() const {
            return m_layer;
        }

        void LayerListBoxLayerItem::refresh() {
            m_nameText->setText(QString::fromStdString(m_layer->name()));
            if (lock(m_document)->currentLayer() == m_layer)
                m_nameText->setStyleSheet("font-weight: bold");
            else
                m_nameText->setStyleSheet("");

            const QString info = tr("%1 %2").arg(m_layer->childCount()).arg(QString::fromStdString(StringUtils::safePlural(m_layer->childCount(), "object", "objects")));
            m_infoText->setText(info);
        }

        void LayerListBoxLayerItem::OnToggleVisible() {
            emit LAYER_TOGGLE_VISIBLE_EVENT(m_layer);
        }

        void LayerListBoxLayerItem::OnToggleLocked() {
            emit LAYER_TOGGLE_LOCKED_EVENT(m_layer);
        }

        void LayerListBoxLayerItem::updateButtons() {
            m_lockButton->setChecked(m_layer->locked());
            m_hiddenButton->setChecked(m_layer->hidden());

            MapDocumentSPtr document = lock(m_document);
            m_lockButton->setEnabled(m_layer->locked() || m_layer != document->currentLayer());
            m_hiddenButton->setEnabled(m_layer->hidden() || m_layer != document->currentLayer());
        }

        // LayerListBox

        LayerListBox::LayerListBox(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            createGui();
            bindObservers();
            bindEvents();
        }

        LayerListBox::~LayerListBox() {
            unbindObservers();
        }

        Model::Layer* LayerListBox::selectedLayer() const {
            if (m_list->selectedItems().isEmpty())
                return nullptr;

            const Model::World* world = lock(m_document)->world();
            const Model::LayerList layers = world->allLayers();

            const size_t index = static_cast<size_t>(m_list->currentRow());
            ensure(index < layers.size(), "index out of range");
            return layers[index];
        }



        void LayerListBox::setSelectedLayer(Model::Layer* layer) {
            m_list->clearSelection();

            const int count = m_list->count();
            for (int i = 0; i < count; ++i) {
                QListWidgetItem* item = m_list->item(i);
                LayerListBoxLayerItem* widget = dynamic_cast<LayerListBoxLayerItem*>(m_list->itemWidget(item));

                if (widget->layer() == layer) {
                    m_list->setItemSelected(item, true);
                    break;
                }
            }
        }

        void LayerListBox::createGui() {
            m_list = new QListWidget();
            m_list->setSelectionMode(QAbstractItemView::SingleSelection);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(m_list, 1);
            setLayout(layout);
        }

        void LayerListBox::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasClearedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->currentLayerDidChangeNotifier.addObserver(this, &LayerListBox::currentLayerDidChange);
            document->nodesWereAddedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
        }

        void LayerListBox::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasClearedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->currentLayerDidChangeNotifier.removeObserver(this, &LayerListBox::currentLayerDidChange);
                document->nodesWereAddedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
            }
        }

        void LayerListBox::documentDidChange(MapDocument* document) {
            refreshList();
        }

        void LayerListBox::nodesDidChange(const Model::NodeList& nodes) {
            refreshList();
        }

        void LayerListBox::currentLayerDidChange(const Model::Layer* layer) {
            refreshList();
        }

        /*
         * FIXME:
        void LayerListBox::OnRightClick() {
            Model::Layer* layer = selectedLayer();
            if (layer != nullptr) {
                emit LAYER_RIGHT_CLICK_EVENT(layer);
            }
        }
         */

        void LayerListBox::bindEvents() {
            connect(m_list, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* currentItem, QListWidgetItem* previousItem){
                Model::Layer* layer = layerForItem(currentItem);
                emit LAYER_SELECTED_EVENT(layer);
            });
            connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item){
                Model::Layer* layer = layerForItem(item);
                emit LAYER_SET_CURRENT_EVENT(layer);
            });
            //Bind(wxEVT_LISTBOX_RCLICK, &LayerListBox::OnRightClick, this);
        }

        void LayerListBox::refreshList() {
            m_list->clear();

            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();

            for (Model::Layer* layer : world->allLayers()) {
                auto* layerWidget = new LayerListBoxLayerItem(nullptr, document, layer);
                auto* item = new QListWidgetItem();

                item->setSizeHint(layerWidget->minimumSizeHint());
                //item->setSizeHint(QSize(-1, layerWidget->minimumSizeHint().height()));

                m_list->addItem(item);
                m_list->setItemWidget(item, layerWidget);
            }
        }

        Model::Layer* LayerListBox::layerForItem(QListWidgetItem* item) {
            auto* widget = dynamic_cast<LayerListBoxLayerItem*>(m_list->itemWidget(item));
            if (widget == nullptr) {
                return nullptr;
            }

            return widget->layer();
        }
    }
}
