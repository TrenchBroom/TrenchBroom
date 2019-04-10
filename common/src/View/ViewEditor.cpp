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

#include "ViewEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/EditorContext.h"
#include "Model/Game.h"
#include "Model/Tag.h"
#include "View/BorderLine.h"
#include "View/BorderPanel.h"
#include "View/MapDocument.h"
#include "View/MapViewConfig.h"
#include "View/PopupButton.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        // EntityDefinitionCheckBoxList

        EntityDefinitionCheckBoxList::EntityDefinitionCheckBoxList(QWidget* parent, Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext) :
        QWidget(parent),
        m_entityDefinitionManager(entityDefinitionManager),
        m_editorContext(editorContext) {
            createGui();
            refresh();
        }

        void EntityDefinitionCheckBoxList::refresh() {
            size_t defIndex = 0;
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();

                if (!definitions.empty()) {
                    const bool firstHidden = m_editorContext.entityDefinitionHidden(definitions[0]);
                    bool mixed = false;
                    for (size_t j = 0; j < definitions.size(); ++j) {
                        const bool hidden = m_editorContext.entityDefinitionHidden(definitions[j]);
                        mixed |= (hidden != firstHidden);
                        m_defCheckBoxes[defIndex++]->setChecked(!hidden);
                    }

                    if (mixed)
                        m_groupCheckBoxes[i]->setCheckState(Qt::PartiallyChecked);
                    else
                        m_groupCheckBoxes[i]->setChecked(!firstHidden);
                    m_groupCheckBoxes[i]->setEnabled(true);
                } else {
                    m_groupCheckBoxes[i]->setChecked(true);
                    m_groupCheckBoxes[i]->setEnabled(false);
                }
            }
        }

        void EntityDefinitionCheckBoxList::OnGroupCheckBoxChanged(const size_t groupIndex, const bool checked) {
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            ensure(groupIndex < m_entityDefinitionManager.groups().size(), "index out of range");
            const Assets::EntityDefinitionGroup& group = groups[groupIndex];

            const Assets::EntityDefinitionList& definitions = group.definitions();
            for (size_t i = 0; i < definitions.size(); ++i) {
                const Assets::EntityDefinition* definition = definitions[i];
                m_editorContext.setEntityDefinitionHidden(definition, !checked);
            }

            refresh();
        }

        void EntityDefinitionCheckBoxList::OnDefCheckBoxChanged(const Assets::EntityDefinition* definition, const bool checked) {
            m_editorContext.setEntityDefinitionHidden(definition, !checked);
            refresh();
        }

        void EntityDefinitionCheckBoxList::OnShowAllClicked() {
            hideAll(false);
        }

        void EntityDefinitionCheckBoxList::OnHideAllClicked() {
            hideAll(true);
        }

        void EntityDefinitionCheckBoxList::hideAll(const bool hidden) {
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                for (size_t j = 0; j < definitions.size(); ++j) {
                    const Assets::EntityDefinition* definition = definitions[j];
                    m_editorContext.setEntityDefinitionHidden(definition, hidden);
                }
            }
        }

        void EntityDefinitionCheckBoxList::createGui() {
            QScrollArea* scrollWindow = new QScrollArea();

            auto* scrollWindowSizer = new QVBoxLayout();
            scrollWindowSizer->addSpacing(1);
            const Assets::EntityDefinitionGroup::List& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const Assets::EntityDefinitionList& definitions = group.definitions();
                const String& groupName = group.displayName();

                // Checkbox for the prefix, e.g. "func"
                auto* groupCB = new QCheckBox(QString::fromStdString(groupName));
                makeEmphasized(groupCB);
                connect(groupCB, &QAbstractButton::clicked, this, [this, i](bool checked){
                    this->OnGroupCheckBoxChanged(i, checked);
                });
                m_groupCheckBoxes.push_back(groupCB);

                scrollWindowSizer->addWidget(groupCB);

                Assets::EntityDefinitionList::const_iterator defIt, defEnd;
                for (defIt = std::begin(definitions), defEnd = std::end(definitions); defIt != defEnd; ++defIt) {
                    Assets::EntityDefinition* definition = *defIt;
                    const String defName = definition->name();

                    auto* defCB = new QCheckBox(QString::fromStdString(defName));
                    defCB->setStyleSheet("margin-left: 11px");

                    connect(defCB, &QAbstractButton::clicked, this, [this, definition](bool checked){
                        this->OnDefCheckBoxChanged(definition, checked);
                    });

                    m_defCheckBoxes.push_back(defCB);
                    scrollWindowSizer->addWidget(defCB);
                }
            }

            scrollWindowSizer->addSpacing(1);

            QWidget* scrollCanvas = new QWidget();
            scrollCanvas->setLayout(scrollWindowSizer);
            scrollWindow->setWidget(scrollCanvas);

            // FIXME:
            //scrollWindow->SetScrollRate(1, checkBoxHeight);

            auto* showAllButton = new QPushButton(tr("Show all"));
            makeEmphasized(showAllButton);
            auto* hideAllButton = new QPushButton(tr("Hide all"));
            makeEmphasized(hideAllButton);

            connect(showAllButton, &QAbstractButton::clicked, this, &EntityDefinitionCheckBoxList::OnShowAllClicked);
            connect(hideAllButton, &QAbstractButton::clicked, this, &EntityDefinitionCheckBoxList::OnHideAllClicked);

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addStretch(1);
            buttonSizer->addSpacing(LayoutConstants::NarrowHMargin);
            buttonSizer->addWidget(showAllButton);
            buttonSizer->addSpacing(LayoutConstants::NarrowHMargin);
            buttonSizer->addWidget(hideAllButton);
            buttonSizer->addSpacing(LayoutConstants::NarrowHMargin);
            buttonSizer->addStretch(1);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->addWidget(scrollWindow, 1);
            outerSizer->addLayout(buttonSizer);
            setLayout(outerSizer);
        }

        // ViewEditor

        ViewEditor::ViewEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            bindObservers();
        }

        ViewEditor::~ViewEditor() {
            unbindObservers();
        }

        void ViewEditor::OnShowEntityClassnamesChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEntityClassnames(checked);
        }

        void ViewEditor::OnShowGroupBoundsChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowGroupBounds(checked);
        }

        void ViewEditor::OnShowBrushEntityBoundsChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowBrushEntityBounds(checked);
        }

        void ViewEditor::OnShowPointEntityBoundsChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityBounds(checked);
        }

        void ViewEditor::OnShowPointEntitiesChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowPointEntities(checked);
        }

        void ViewEditor::OnShowPointEntityModelsChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityModels(checked);
        }

        void ViewEditor::OnShowBrushesChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowBrushes(checked);
        }

        void ViewEditor::OnShowTagChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);

            Model::Tag::TagType hiddenTags = 0;
            const auto& tags = document->smartTags();

            auto tagIt = std::begin(tags);
            auto boxIt = std::begin(m_tagCheckBoxes);
            while (tagIt != std::end(tags) && boxIt != std::end(m_tagCheckBoxes)) {
                const auto& tag = *tagIt;
                QCheckBox* checkBox = *boxIt;
                if (!checkBox->isChecked()) {
                    hiddenTags |= tag.type();
                }
                ++tagIt; ++boxIt;
            }

            auto& editorContext = document->editorContext();
            editorContext.setHiddenTags(hiddenTags);
        }

        void ViewEditor::OnFaceRenderModeChanged(const int id) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();

            switch (id) {
                case 1:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Flat);
                    break;
                case 2:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Skip);
                    break;
                default:
                    config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Textured);
                    break;
            }
        }

        void ViewEditor::OnShadeFacesChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShadeFaces(checked);
        }

        void ViewEditor::OnShowFogChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowFog(checked);
        }

        void ViewEditor::OnShowEdgesChanged(const bool checked) {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEdges(checked);
        }

        void ViewEditor::OnEntityLinkModeChanged(const int id) {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();

            switch (id) {
                case 0:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_All);
                    break;
                case 1:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Transitive);
                    break;
                case 2:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Direct);
                    break;
                default:
                    editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_None);
                    break;
            }
        }

        void ViewEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->editorContextDidChangeNotifier.addObserver(this, &ViewEditor::editorContextDidChange);
            document->mapViewConfigDidChangeNotifier.addObserver(this, &ViewEditor::mapViewConfigDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &ViewEditor::entityDefinitionsDidChange);
        }

        void ViewEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->editorContextDidChangeNotifier.removeObserver(this, &ViewEditor::editorContextDidChange);
                document->mapViewConfigDidChangeNotifier.removeObserver(this, &ViewEditor::mapViewConfigDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &ViewEditor::entityDefinitionsDidChange);
            }
        }

        void ViewEditor::documentWasNewedOrLoaded(MapDocument* document) {
            createGui();
            refreshGui();
        }

        void ViewEditor::editorContextDidChange() {
            refreshGui();
        }

        void ViewEditor::mapViewConfigDidChange() {
            refreshGui();
        }

        void ViewEditor::entityDefinitionsDidChange() {
            createGui();
            refreshGui();
        }

        void ViewEditor::createGui() {
            setLayout(nullptr);

            auto* sizer = new QGridLayout();
            sizer->addWidget(createEntityDefinitionsPanel(this), 0,0,3,1);
            sizer->addWidget(createEntitiesPanel(this),          0,1);
            sizer->addWidget(createBrushesPanel(this),           1,1);
            sizer->addWidget(createRendererPanel(this),          2,1);

            setLayout(sizer);
        }

        QWidget* ViewEditor::createEntityDefinitionsPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Entity Definitions", false);

            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& entityDefinitionManager = document->entityDefinitionManager();

            Model::EditorContext& editorContext = document->editorContext();
            m_entityDefinitionCheckBoxList = new EntityDefinitionCheckBoxList(panel->getPanel(), entityDefinitionManager, editorContext);

            auto* panelSizer = new QVBoxLayout();
            panelSizer->addWidget(m_entityDefinitionCheckBoxList, 1);
            m_entityDefinitionCheckBoxList->setMinimumSize(250, -1);
            panel->getPanel()->setLayout(panelSizer);

            return panel;
        }

        QWidget* ViewEditor::createEntitiesPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Entities", false);

            m_showEntityClassnamesCheckBox = new QCheckBox(tr("Show entity classnames"));
            m_showGroupBoundsCheckBox = new QCheckBox(tr("Show group bounds"));
            m_showBrushEntityBoundsCheckBox = new QCheckBox(tr("Show brush entity bounds"));
            m_showPointEntityBoundsCheckBox = new QCheckBox(tr("Show point entity bounds"));

            m_showPointEntitiesCheckBox = new QCheckBox(tr("Show point entities"));
            m_showPointEntityModelsCheckBox = new QCheckBox(tr("Show point entity models"));

            connect(m_showEntityClassnamesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowEntityClassnamesChanged);
            connect(m_showGroupBoundsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowGroupBoundsChanged);
            connect(m_showBrushEntityBoundsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowBrushEntityBoundsChanged);
            connect(m_showPointEntityBoundsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowPointEntityBoundsChanged);
            connect(m_showPointEntitiesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowPointEntitiesChanged);
            connect(m_showPointEntityModelsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowPointEntityModelsChanged);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(m_showEntityClassnamesCheckBox);
            sizer->addWidget(m_showGroupBoundsCheckBox);
            sizer->addWidget(m_showBrushEntityBoundsCheckBox);
            sizer->addWidget(m_showPointEntityBoundsCheckBox);
            sizer->addWidget(m_showPointEntitiesCheckBox);
            sizer->addWidget(m_showPointEntityModelsCheckBox);

            panel->getPanel()->setLayout(sizer);
            return panel;
        }

        QWidget* ViewEditor::createBrushesPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Brushes", false);
            auto* inner = panel->getPanel();
            createTagFilter(inner);

            m_showBrushesCheckBox = new QCheckBox(tr("Show brushes"));
            connect(m_showBrushesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowBrushesChanged);

            auto* innerLayout = qobject_cast<QBoxLayout*>(inner->layout());
            ensure(innerLayout != nullptr, "inner sizer is null");
            innerLayout->insertWidget(0, m_showBrushesCheckBox);

            return panel;
        }

        void ViewEditor::createTagFilter(QWidget* parent) {
            m_tagCheckBoxes.clear();

            MapDocumentSPtr document = lock(m_document);
            const auto& tags = document->smartTags();
            if (tags.empty()) {
                createEmptyTagFilter(parent);
            } else {
                createTagFilter(parent, tags);
            }
        }

        void ViewEditor::createEmptyTagFilter(QWidget* parent) {
            auto* msg = new QLabel(tr("No tags found"));
            makeInfo(msg); // msg->SetForegroundColour(*wxLIGHT_GREY);

            auto* sizer = new QHBoxLayout();
            sizer->addSpacing(LayoutConstants::WideHMargin);
            sizer->addWidget(msg);
            sizer->addSpacing(LayoutConstants::WideHMargin);

            parent->setLayout(sizer);
        }

        void ViewEditor::createTagFilter(QWidget* parent, const std::list<Model::SmartTag>& tags) {
            assert(!tags.empty());

            auto* sizer = new QVBoxLayout();
            for (const auto& tag : tags) {
                const QString label = QString::fromLatin1("Show %1").arg(QString::fromStdString(tag.name()).toLower());

                auto* checkBox = new QCheckBox(label);
                m_tagCheckBoxes.push_back(checkBox);

                sizer->addWidget(checkBox);
                connect(checkBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowTagChanged);
            }
            parent->setLayout(sizer);
        }

        QWidget* ViewEditor::createRendererPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel(parent, "Renderer", false);
            QWidget* inner = panel->getPanel();

            const QList<QString> FaceRenderModes = { "Show textures", "Hide textures", "Hide faces" };
            m_renderModeRadioGroup = new QButtonGroup(this);
            for (int i = 0; i < FaceRenderModes.length(); ++i) {
                const QString& label = FaceRenderModes.at(i);

                auto* radio = new QRadioButton(label);
                m_renderModeRadioGroup->addButton(radio, i);
            }

            m_shadeFacesCheckBox = new QCheckBox(tr("Shade faces"));
            m_showFogCheckBox = new QCheckBox(tr("Use fog"));
            m_showEdgesCheckBox = new QCheckBox(tr("Show edges"));

            const QList<QString> EntityLinkModes = { "Show all entity links", "Show transitively selected entity links", "Show directly selected entity links", "Hide entity links" };
            m_entityLinkRadioGroup = new QButtonGroup(this);
            for (int i = 0; i < EntityLinkModes.length(); ++i) {
                const QString& label = EntityLinkModes.at(i);

                auto* radio = new QRadioButton(label);
                m_entityLinkRadioGroup->addButton(radio, i);
            }

            connect(m_shadeFacesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShadeFacesChanged);
            connect(m_showFogCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowFogChanged);
            connect(m_showEdgesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::OnShowEdgesChanged);

            connect(m_renderModeRadioGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ViewEditor::OnFaceRenderModeChanged);
            connect(m_entityLinkRadioGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ViewEditor::OnEntityLinkModeChanged);

            auto* sizer = new QVBoxLayout();
            for (auto* button : m_renderModeRadioGroup->buttons()) {
                sizer->addWidget(button);
            }
            sizer->addWidget(m_shadeFacesCheckBox);
            sizer->addWidget(m_showFogCheckBox);
            sizer->addWidget(m_showEdgesCheckBox);
            for (auto* button : m_entityLinkRadioGroup->buttons()) {
                sizer->addWidget(button);
            }

            inner->setLayout(sizer);
            return panel;
        }

        void ViewEditor::refreshGui() {
            refreshEntityDefinitionsPanel();
            refreshEntitiesPanel();
            refreshBrushesPanel();
            refreshRendererPanel();
        }

        void ViewEditor::refreshEntityDefinitionsPanel() {
            m_entityDefinitionCheckBoxList->refresh();
        }

        void ViewEditor::refreshEntitiesPanel() {
            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& config = document->mapViewConfig();

            m_showEntityClassnamesCheckBox->setChecked(config.showEntityClassnames());
            m_showGroupBoundsCheckBox->setChecked(config.showGroupBounds());
            m_showBrushEntityBoundsCheckBox->setChecked(config.showBrushEntityBounds());
            m_showPointEntityBoundsCheckBox->setChecked(config.showPointEntityBounds());
            m_showPointEntitiesCheckBox->setChecked(config.showPointEntities());
            m_showPointEntityModelsCheckBox->setChecked(config.showPointEntityModels());
        }

        void ViewEditor::refreshBrushesPanel() {
            MapDocumentSPtr document = lock(m_document);

            const MapViewConfig& config = document->mapViewConfig();
            m_showBrushesCheckBox->setChecked(config.showBrushes());

            Model::EditorContext& editorContext = document->editorContext();
            const Model::Tag::TagType hiddenTags = editorContext.hiddenTags();

            const auto& tags = document->smartTags();
            auto tagIt = std::begin(tags);
            auto boxIt = std::begin(m_tagCheckBoxes);
            while (tagIt != std::end(tags) && boxIt != std::end(m_tagCheckBoxes)) {
                const Model::Tag& tag = *tagIt;;
                QCheckBox* checkBox = *boxIt;
                checkBox->setChecked((tag.type() & hiddenTags) == 0);
                ++tagIt; ++boxIt;
            }
        }

        void ViewEditor::refreshRendererPanel() {
            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& config = document->mapViewConfig();
            Model::EditorContext& editorContext = document->editorContext();

            checkButtonInGroup(m_renderModeRadioGroup, static_cast<int>(config.faceRenderMode()), true);
            m_shadeFacesCheckBox->setChecked(config.shadeFaces());
            m_showFogCheckBox->setChecked(config.showFog());
            m_showEdgesCheckBox->setChecked(config.showEdges());
            checkButtonInGroup(m_entityLinkRadioGroup, static_cast<int>(editorContext.entityLinkMode()), true);
        }

        ViewPopupEditor::ViewPopupEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_button(nullptr),
        m_editor(nullptr) {
            m_button = new PopupButton(nullptr, tr("View"));
            m_button->setToolTip(tr("Click to edit view settings"));

            auto* editorContainer = new BorderPanel(m_button->GetPopupWindow());
            m_editor = new ViewEditor(editorContainer, document);

            auto* containerSizer = new QVBoxLayout();
            containerSizer->addWidget(m_editor);
            editorContainer->setLayout(containerSizer);

            auto* popupSizer = new QVBoxLayout();
            popupSizer->addWidget(editorContainer);
            m_button->GetPopupWindow()->setLayout(popupSizer);

            auto* sizer = new QHBoxLayout();
            sizer->addWidget(m_button, Qt::AlignVCenter);
            setLayout(sizer);
        }
    }
}
