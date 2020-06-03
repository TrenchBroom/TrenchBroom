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

#include <optional>
#include <utility>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
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
        m_document(document),
        m_softBoundsDisabled(nullptr),
        m_softBoundsFromGame(nullptr),
        m_softBoundsFromGameMinLabel(nullptr),
        m_softBoundsFromGameMaxLabel(nullptr),
        m_softBoundsFromMap(nullptr),
        m_softBoundsFromMapMinEdit(nullptr),
        m_softBoundsFromMapMaxEdit(nullptr) {
            createGui();
            bindObservers();
        }

        MapPropertiesEditor::~MapPropertiesEditor() {
            unbindObservers();
        }

        static std::optional<vm::vec3> parseVec(const QString& qString) {
            const std::string string = qString.toStdString();
            
            if (vm::can_parse<double, 3u>(string)) {
                return { vm::parse<double, 3u>(string) };
            }

            if (vm::can_parse<double, 1u>(string)) {
                const double fillValue = vm::parse<double, 1u>(string).x();
                return { vm::vec3::fill(fillValue) };
            }

            return std::nullopt;
        }

        std::optional<vm::bbox3> MapPropertiesEditor::parseLineEdits() {
            const auto min = parseVec(m_softBoundsFromMapMinEdit->text());
            const auto max = parseVec(m_softBoundsFromMapMaxEdit->text());

            if (!min.has_value() || !max.has_value()) {
                return std::nullopt;
            }
            return vm::bbox3(*min, *max);
        }

        void MapPropertiesEditor::createGui() {
            m_softBoundsDisabled = new QRadioButton(tr("Soft bounds disabled"));

            m_softBoundsFromGame = new QRadioButton(tr("Game default bounds:"));
            m_softBoundsFromGameMinLabel = new QLabel();
            m_softBoundsFromGameMaxLabel = new QLabel();

            auto* softBoundsFromGameValueLayout = new QHBoxLayout();
            softBoundsFromGameValueLayout->addWidget(new QLabel(tr("Min:")));
            softBoundsFromGameValueLayout->addWidget(m_softBoundsFromGameMinLabel);
            softBoundsFromGameValueLayout->addWidget(new QLabel(tr("Min:")));
            softBoundsFromGameValueLayout->addWidget(m_softBoundsFromGameMaxLabel);

            m_softBoundsFromMap = new QRadioButton(tr("Custom bounds:"));
            m_softBoundsFromMapMinEdit = new QLineEdit();
            m_softBoundsFromMapMaxEdit = new QLineEdit();

            auto* softBoundsFromMapValueLayout = new QHBoxLayout();
            softBoundsFromMapValueLayout->addWidget(new QLabel(tr("Min:")));
            softBoundsFromMapValueLayout->addWidget(m_softBoundsFromMapMinEdit);
            softBoundsFromMapValueLayout->addWidget(new QLabel(tr("Min:")));
            softBoundsFromMapValueLayout->addWidget(m_softBoundsFromMapMaxEdit);

            QFormLayout* formLayout = new QFormLayout();
            formLayout->setContentsMargins(0, 0, 0, 0);
            formLayout->setSpacing(0);
            formLayout->addRow(m_softBoundsDisabled);
            formLayout->addRow(m_softBoundsFromGame, softBoundsFromGameValueLayout);
            formLayout->addRow(m_softBoundsFromMap, softBoundsFromMapValueLayout);
            setLayout(formLayout);

            connect(m_softBoundsDisabled, &QAbstractButton::clicked, this, [this](const bool checked) {
                auto document = kdl::mem_lock(m_document);
                if (checked) {
                    document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, vm::bbox3()});
                }
            });
            connect(m_softBoundsFromGame, &QAbstractButton::clicked, this, [this](const bool checked) {
                auto document = kdl::mem_lock(m_document);
                if (checked) {
                    document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Game, vm::bbox3()});
                }
            });
            connect(m_softBoundsFromMap, &QAbstractButton::clicked, this, [this](const bool checked) {
                auto document = kdl::mem_lock(m_document);

                m_softBoundsFromMapMinEdit->setEnabled(true);
                m_softBoundsFromMapMaxEdit->setEnabled(true);

                if (checked) {
                    const std::optional<vm::bbox3> parsed = parseLineEdits();
                    // Only commit the change to the document right now if both text fields can be parsed.
                    // Otherwise, it will be committed below in textEditingFinished once
                    // both text fields have a valid value entered.
                    if (parsed.has_value()) {
                        document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, parsed.value()});
                    }
                }
            });

            const auto textEditingFinished = [this]() {
                auto document = kdl::mem_lock(m_document);

                const std::optional<vm::bbox3> parsed = parseLineEdits();
                if (parsed.has_value()) {
                    document->setSoftMapBounds({Model::Game::SoftMapBoundsType::Map, parsed.value()});
                }
            };
            connect(m_softBoundsFromMapMinEdit, &QLineEdit::textEdited, this, textEditingFinished);
            connect(m_softBoundsFromMapMaxEdit, &QLineEdit::textEdited, this, textEditingFinished);

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

        static QString formatVec(const vm::vec3& vec) {            
            std::stringstream str;
            if (vec.x() == vec.y() && vec.y() == vec.z()) {
                // Just print the first component to save space
                str << vec.x();
            } else {
                str << vec;
            }
            return QString::fromStdString(str.str());
        }

        /**
         * Refresh the UI from the model
         */
        void MapPropertiesEditor::updateGui() {
            auto document = kdl::mem_lock(m_document);
            if (!document) {
                return;
            }

            auto game = document->game();
            if (game == nullptr) {
                return;
            }

            const vm::bbox3 gameBounds = game->softMapBounds();
            m_softBoundsFromGameMinLabel->setText(formatVec(gameBounds.min));
            m_softBoundsFromGameMaxLabel->setText(formatVec(gameBounds.max));

            // FIXME: disambiguate 0 from "no bounds" by switching back to an optional
            // FIXME: serialize "no bounds" as "none" rather than ""
            const auto bounds = document->softMapBounds();            

            if (bounds.first == Model::Game::SoftMapBoundsType::Map && bounds.second.is_empty()) {
                m_softBoundsDisabled->setChecked(true);

                m_softBoundsFromMapMinEdit->setEnabled(false);
                m_softBoundsFromMapMaxEdit->setEnabled(false);
            } else if (bounds.first == Model::Game::SoftMapBoundsType::Map && !bounds.second.is_empty()) {
                m_softBoundsFromMap->setChecked(true);

                m_softBoundsFromMapMinEdit->setEnabled(true);
                m_softBoundsFromMapMaxEdit->setEnabled(true);

                m_softBoundsFromMapMinEdit->setText(formatVec(bounds.second.min));
                m_softBoundsFromMapMaxEdit->setText(formatVec(bounds.second.max));
            } else {
                m_softBoundsFromGame->setChecked(true);

                m_softBoundsFromMapMinEdit->setEnabled(false);
                m_softBoundsFromMapMaxEdit->setEnabled(false);
            }                    
        }
    }
}
