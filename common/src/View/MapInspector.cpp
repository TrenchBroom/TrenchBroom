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

#include "MapInspector.h"

#include "Model/EntityAttributes.h"
#include "Model/WorldNode.h"
#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/LayerEditor.h"
#include "View/MapDocument.h"
#include "View/ModEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>

#include <vecmath/vec_io.h>

#include <utility>

#include <QDebug>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>

namespace TrenchBroom {
    namespace View {
        // MapInspector

        MapInspector::MapInspector(std::weak_ptr<MapDocument> document, QWidget* parent) :
        TabBookPage(parent) {
            createGui(document);
        }

        void MapInspector::createGui(std::weak_ptr<MapDocument> document) {
            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);

            sizer->addWidget(createLayerEditor(document), 1);
            sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            sizer->addWidget(createMapProperties(document), 0);
            sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            sizer->addWidget(createModEditor(document), 0);
            setLayout(sizer);
        }

        QWidget* MapInspector::createLayerEditor(std::weak_ptr<MapDocument> document) {
            TitledPanel* titledPanel = new TitledPanel(tr("Layers"));
            LayerEditor* layerEditor = new LayerEditor(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(layerEditor, 1);
            titledPanel->getPanel()->setLayout(sizer);

            return titledPanel;
        }

        QWidget* MapInspector::createMapProperties(std::weak_ptr<MapDocument> document) {
            CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(tr("Map Properties"), false);
            auto* editor = new MapPropertiesEditor(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(editor, 1);
            titledPanel->getPanel()->setLayout(sizer);

            return titledPanel;
        }

        QWidget* MapInspector::createModEditor(std::weak_ptr<MapDocument> document) {
            CollapsibleTitledPanel* titledPanel = new CollapsibleTitledPanel(tr("Mods"), false);
            ModEditor* modEditor = new ModEditor(document);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(modEditor, 1);
            titledPanel->getPanel()->setLayout(sizer);

            return titledPanel;
        }

        // MapPropertiesEditor

        MapPropertiesEditor::MapPropertiesEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document) {
            createGui();
            bindObservers();
        }

        MapPropertiesEditor::~MapPropertiesEditor() {
            unbindObservers();
        }

        static std::optional<vm::bbox3> parseBounds(const std::string& string) {
            // FIXME: duplicated in GameConfigParser
            if (!vm::can_parse<double, 3u>(string)) {
                return std::nullopt;
            }

            const auto vec = vm::parse<double, 3u>(string);
            return { vm::bbox3(-0.5 * vec, 0.5 * vec) };
        }

        static QString formatBounds(const vm::bbox3& bounds) {
            std::stringstream str;
            str << bounds.size();
            return QString::fromStdString(str.str());
        }

        void MapPropertiesEditor::createGui() {
            //auto* group = new QButtonGroup();

            m_softBoundsDisabled = new QRadioButton(tr("Soft bounds disabled"));
            m_softBoundsFromGame = new QRadioButton(tr("Game default bounds"));
            m_softBoundsFromMap = new QRadioButton(tr("Custom bounds:"));

            m_sizeBox = new QLineEdit();

            QFormLayout* formLayout = new QFormLayout();
            formLayout->setContentsMargins(0, 0, 0, 0);
            formLayout->setSpacing(0);
            formLayout->addRow(m_softBoundsDisabled);
            formLayout->addRow(m_softBoundsFromGame);
            formLayout->addRow(m_softBoundsFromMap, m_sizeBox);
            setLayout(formLayout);

            connect(m_softBoundsDisabled, &QAbstractButton::clicked, this, [this](const bool checked) {
                // This signal happens in response to user input only
                auto document = kdl::mem_lock(m_document);
                if (checked) {
                    document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, vm::bbox3()});
                }
            });
            connect(m_softBoundsFromGame, &QAbstractButton::clicked, this, [this](const bool checked) {
                // This signal happens in response to user input only
                auto document = kdl::mem_lock(m_document);
                if (checked) {
                    document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Game, vm::bbox3()});
                }
            });
            connect(m_softBoundsFromMap, &QAbstractButton::clicked, this, [this](const bool /*checked*/) {
                // This signal happens in response to user input only
                auto document = kdl::mem_lock(m_document);
                // if (checked) {
                //     document->setMapSoftBounds(parseBounds(m_sizeBox->text().toStdString()));                    
                // }
            });

            connect(m_sizeBox, &QLineEdit::editingFinished, this, [this]() {
                // This signal happens in response to user input only
                auto document = kdl::mem_lock(m_document);
                // if (document) {
                //     document->setMapSoftBounds(parseBounds(m_sizeBox->text().toStdString()));
                // }
            });

            updateGui();
        }

        void MapPropertiesEditor::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &MapPropertiesEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &MapPropertiesEditor::documentWasLoaded);
            document->nodesDidChangeNotifier.addObserver(this, &MapPropertiesEditor::nodesDidChange);
        }

        void MapPropertiesEditor::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &MapPropertiesEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &MapPropertiesEditor::documentWasLoaded);
                document->nodesDidChangeNotifier.removeObserver(this, &MapPropertiesEditor::nodesDidChange);
            }
        }

        void MapPropertiesEditor::documentWasNewed(MapDocument*) {
            updateGui();
        }

        void MapPropertiesEditor::documentWasLoaded(MapDocument*) {
            updateGui();
        }

        void MapPropertiesEditor::nodesDidChange(const std::vector<Model::Node*>& nodes) {
            auto document = kdl::mem_lock(m_document);
            if (!document) {
                return;
            }

            for (Model::Node* node : nodes) {
                if (node == document->world()) {
                    updateGui();
                    return;
                }
            }
        }

        /**
         * Refresh the UI from the model
         */
        void MapPropertiesEditor::updateGui() {
            // checkbox is checked iff Model::AttributeNames::SoftMaxMapSize key is set

            auto document = kdl::mem_lock(m_document);
            if (!document) {
                return;
            }

            
            const auto bounds = document->softMapBounds();
            const QString boundsQString = formatBounds(bounds.second);

            if (bounds.first == Model::Game::SoftMapBoundsType::Map && bounds.second.is_empty()) {
                m_softBoundsDisabled->setChecked(true);
                m_sizeBox->setEnabled(false);
            } else if (bounds.first == Model::Game::SoftMapBoundsType::Map && !bounds.second.is_empty()) {
                m_softBoundsFromMap->setChecked(true);
                m_sizeBox->setEnabled(true);
                m_sizeBox->setText(boundsQString); 
            } else {
                m_softBoundsFromGame->setChecked(true);
                m_sizeBox->setEnabled(false);
            }
 
            //qDebug() << "MapPropertiesEditor::updateGui:" << boundsQString << "set:" << hasBoundsSet;
                       
        }
    }
}
