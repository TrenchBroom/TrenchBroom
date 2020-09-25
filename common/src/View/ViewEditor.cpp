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
#include "Model/TagType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/BorderPanel.h"
#include "View/MapDocument.h"
#include "View/PopupButton.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <vector>

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        // EntityDefinitionCheckBoxList

        EntityDefinitionCheckBoxList::EntityDefinitionCheckBoxList(Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext, QWidget* parent) :
        QWidget(parent),
        m_entityDefinitionManager(entityDefinitionManager),
        m_editorContext(editorContext) {
            createGui();
            refresh();
        }

        void EntityDefinitionCheckBoxList::refresh() {
            size_t defIndex = 0;
            const std::vector<Assets::EntityDefinitionGroup>& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const std::vector<Assets::EntityDefinition*>& definitions = group.definitions();

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

        void EntityDefinitionCheckBoxList::groupCheckBoxChanged(size_t groupIndex, bool checked) {
            const std::vector<Assets::EntityDefinitionGroup>& groups = m_entityDefinitionManager.groups();
            ensure(groupIndex < m_entityDefinitionManager.groups().size(), "index out of range");
            const Assets::EntityDefinitionGroup& group = groups[groupIndex];

            const std::vector<Assets::EntityDefinition*>& definitions = group.definitions();
            for (size_t i = 0; i < definitions.size(); ++i) {
                const Assets::EntityDefinition* definition = definitions[i];
                m_editorContext.setEntityDefinitionHidden(definition, !checked);
            }

            refresh();
        }

        void EntityDefinitionCheckBoxList::defCheckBoxChanged(const Assets::EntityDefinition* definition, bool checked) {
            m_editorContext.setEntityDefinitionHidden(definition, !checked);
            refresh();
        }

        void EntityDefinitionCheckBoxList::showAllClicked() {
            hideAll(false);
        }

        void EntityDefinitionCheckBoxList::hideAllClicked() {
            hideAll(true);
        }

        void EntityDefinitionCheckBoxList::hideAll(const bool hidden) {
            const std::vector<Assets::EntityDefinitionGroup>& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const std::vector<Assets::EntityDefinition*>& definitions = group.definitions();
                for (size_t j = 0; j < definitions.size(); ++j) {
                    const Assets::EntityDefinition* definition = definitions[j];
                    m_editorContext.setEntityDefinitionHidden(definition, hidden);
                }
            }
        }

        void EntityDefinitionCheckBoxList::createGui() {
            auto* scrollWidgetLayout = new QVBoxLayout();
            scrollWidgetLayout->setContentsMargins(0, 0, 0, 0);
            scrollWidgetLayout->setSpacing(0);
            scrollWidgetLayout->addSpacing(1);

            const std::vector<Assets::EntityDefinitionGroup>& groups = m_entityDefinitionManager.groups();
            for (size_t i = 0; i < groups.size(); ++i) {
                const Assets::EntityDefinitionGroup& group = groups[i];
                const std::vector<Assets::EntityDefinition*>& definitions = group.definitions();
                const std::string& groupName = group.displayName();

                // Checkbox for the prefix, e.g. "func"
                auto* groupCB = new QCheckBox(QString::fromStdString(groupName));
                makeEmphasized(groupCB);
                connect(groupCB, &QAbstractButton::clicked, this, [this, i](bool checked){
                    this->groupCheckBoxChanged(i, checked);
                });
                m_groupCheckBoxes.push_back(groupCB);

                scrollWidgetLayout->addWidget(groupCB);

                for (auto defIt = std::begin(definitions), defEnd = std::end(definitions); defIt != defEnd; ++defIt) {
                    Assets::EntityDefinition* definition = *defIt;
                    const std::string defName = definition->name();

                    auto* defCB = new QCheckBox(QString::fromStdString(defName));
                    defCB->setObjectName("entityDefinition_checkboxWidget");

                    connect(defCB, &QAbstractButton::clicked, this, [this, definition](bool checked){
                        this->defCheckBoxChanged(definition, checked);
                    });

                    m_defCheckBoxes.push_back(defCB);
                    scrollWidgetLayout->addWidget(defCB);
                }
            }

            scrollWidgetLayout->addSpacing(1);

            auto* scrollWidget = new QWidget();
            scrollWidget->setLayout(scrollWidgetLayout);

            auto* scrollArea = new QScrollArea();
            scrollArea->setBackgroundRole(QPalette::Base);
            scrollArea->setAutoFillBackground(true);
            scrollArea->setWidget(scrollWidget);

            auto* showAllButton = new QPushButton(tr("Show all"));
            makeEmphasized(showAllButton);
            auto* hideAllButton = new QPushButton(tr("Hide all"));
            makeEmphasized(hideAllButton);

            connect(showAllButton, &QAbstractButton::clicked, this, &EntityDefinitionCheckBoxList::showAllClicked);
            connect(hideAllButton, &QAbstractButton::clicked, this, &EntityDefinitionCheckBoxList::hideAllClicked);

            auto* buttonLayout = new QHBoxLayout();
            buttonLayout->setContentsMargins(0, 0, 0, 0);
            buttonLayout->setSpacing(LayoutConstants::NarrowHMargin);
            buttonLayout->addStretch(1);
            buttonLayout->addWidget(showAllButton);
            buttonLayout->addWidget(hideAllButton);
            buttonLayout->addStretch(1);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(LayoutConstants::MediumVMargin);
            outerLayout->addWidget(scrollArea, 1);
            outerLayout->addLayout(buttonLayout);
            setLayout(outerLayout);
        }

        // ViewEditor

        ViewEditor::ViewEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_showEntityClassnamesCheckBox(nullptr),
        m_showGroupBoundsCheckBox(nullptr),
        m_showBrushEntityBoundsCheckBox(nullptr),
        m_showPointEntityBoundsCheckBox(nullptr),
        m_showPointEntitiesCheckBox(nullptr),
        m_showPointEntityModelsCheckBox(nullptr),
        m_entityDefinitionCheckBoxList(nullptr),
        m_showBrushesCheckBox(nullptr),
        m_renderModeRadioGroup(nullptr),
        m_shadeFacesCheckBox(nullptr),
        m_showFogCheckBox(nullptr),
        m_showEdgesCheckBox(nullptr),
        m_entityLinkRadioGroup(nullptr),
        m_showSoftBoundsCheckBox(nullptr) {
            bindObservers();
        }

        ViewEditor::~ViewEditor() {
            unbindObservers();
        }

        void ViewEditor::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &ViewEditor::documentWasNewedOrLoaded);
            document->editorContextDidChangeNotifier.addObserver(this, &ViewEditor::editorContextDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &ViewEditor::entityDefinitionsDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ViewEditor::preferenceDidChange);
        }

        void ViewEditor::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &ViewEditor::documentWasNewedOrLoaded);
                document->editorContextDidChangeNotifier.removeObserver(this, &ViewEditor::editorContextDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &ViewEditor::entityDefinitionsDidChange);
            }

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &ViewEditor::preferenceDidChange);
        }

        void ViewEditor::documentWasNewedOrLoaded(MapDocument*) {
            createGui();
            refreshGui();
        }

        void ViewEditor::editorContextDidChange() {
            refreshGui();
        }

        void ViewEditor::entityDefinitionsDidChange() {
            createGui();
            refreshGui();
        }

        void ViewEditor::preferenceDidChange(const IO::Path&) {
            refreshGui();
        }

        void ViewEditor::createGui() {
            deleteChildWidgetsLaterAndDeleteLayout(this);

            auto* sizer = new QGridLayout();
            sizer->setContentsMargins(
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin,
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin);
            sizer->setHorizontalSpacing(LayoutConstants::WideHMargin);
            sizer->setVerticalSpacing(LayoutConstants::WideVMargin);
            sizer->addWidget(createEntityDefinitionsPanel(this), 0,0,3,1);
            sizer->addWidget(createEntitiesPanel(this),          0,1);
            sizer->addWidget(createBrushesPanel(this),           1,1);
            sizer->addWidget(createRendererPanel(this),          2,1);

            setLayout(sizer);
        }

        QWidget* ViewEditor::createEntityDefinitionsPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel("Entity Definitions", parent, false);

            auto document = kdl::mem_lock(m_document);
            Assets::EntityDefinitionManager& entityDefinitionManager = document->entityDefinitionManager();

            Model::EditorContext& editorContext = document->editorContext();
            m_entityDefinitionCheckBoxList = new EntityDefinitionCheckBoxList(entityDefinitionManager, editorContext);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_entityDefinitionCheckBoxList, 1);
            m_entityDefinitionCheckBoxList->setMinimumWidth(250);
            panel->getPanel()->setLayout(layout);

            return panel;
        }

        QWidget* ViewEditor::createEntitiesPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel("Entities", parent, false);

            m_showEntityClassnamesCheckBox = new QCheckBox(tr("Show entity classnames"));
            m_showGroupBoundsCheckBox = new QCheckBox(tr("Show group bounds and names"));
            m_showBrushEntityBoundsCheckBox = new QCheckBox(tr("Show brush entity bounds"));
            m_showPointEntityBoundsCheckBox = new QCheckBox(tr("Show point entity bounds"));

            m_showPointEntitiesCheckBox = new QCheckBox(tr("Show point entities"));
            m_showPointEntityModelsCheckBox = new QCheckBox(tr("Show point entity models"));

            connect(m_showEntityClassnamesCheckBox, &QAbstractButton::clicked, this,
                &ViewEditor::showEntityClassnamesChanged);
            connect(m_showGroupBoundsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showGroupBoundsChanged);
            connect(m_showBrushEntityBoundsCheckBox, &QAbstractButton::clicked, this,
                &ViewEditor::showBrushEntityBoundsChanged);
            connect(m_showPointEntityBoundsCheckBox, &QAbstractButton::clicked, this,
                &ViewEditor::showPointEntityBoundsChanged);
            connect(m_showPointEntitiesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showPointEntitiesChanged);
            connect(m_showPointEntityModelsCheckBox, &QAbstractButton::clicked, this,
                &ViewEditor::showPointEntityModelsChanged);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_showEntityClassnamesCheckBox);
            layout->addWidget(m_showGroupBoundsCheckBox);
            layout->addWidget(m_showBrushEntityBoundsCheckBox);
            layout->addWidget(m_showPointEntityBoundsCheckBox);
            layout->addWidget(m_showPointEntitiesCheckBox);
            layout->addWidget(m_showPointEntityModelsCheckBox);

            panel->getPanel()->setLayout(layout);
            return panel;
        }

        QWidget* ViewEditor::createBrushesPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel("Brushes", parent, false);
            auto* inner = panel->getPanel();
            createTagFilter(inner);

            m_showBrushesCheckBox = new QCheckBox(tr("Show brushes"));
            connect(m_showBrushesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showBrushesChanged);

            auto* innerLayout = qobject_cast<QBoxLayout*>(inner->layout());
            ensure(innerLayout != nullptr, "inner sizer is null");
            innerLayout->insertWidget(0, m_showBrushesCheckBox);

            return panel;
        }

        void ViewEditor::createTagFilter(QWidget* parent) {
            m_tagCheckBoxes.clear();

            auto document = kdl::mem_lock(m_document);
            const auto& tags = document->smartTags();
            if (tags.empty()) {
                createEmptyTagFilter(parent);
            } else {
                createTagFilter(parent, tags);
            }
        }

        void ViewEditor::createEmptyTagFilter(QWidget* parent) {
            auto* msg = new QLabel(tr("No tags found"));
            makeInfo(msg);

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, LayoutConstants::WideVMargin, 0, LayoutConstants::WideVMargin);
            layout->setSpacing(0);
            layout->addWidget(msg);

            parent->setLayout(layout);
        }

        void ViewEditor::createTagFilter(QWidget* parent, const std::vector<Model::SmartTag>& tags) {
            assert(!tags.empty());

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            for (const auto& tag : tags) {
                const QString label = QString::fromLatin1("Show %1").arg(QString::fromStdString(tag.name()).toLower());

                auto* checkBox = new QCheckBox(label);
                const Model::TagType::Type tagType = tag.type();

                m_tagCheckBoxes.emplace_back(tagType, checkBox);

                layout->addWidget(checkBox);                
                connect(checkBox, &QAbstractButton::clicked, this, [this, tagType](const bool checked) {
                    showTagChanged(checked, tagType);
                });
            }
            parent->setLayout(layout);
        }

        QWidget* ViewEditor::createRendererPanel(QWidget* parent) {
            TitledPanel* panel = new TitledPanel("Renderer", parent, false);
            QWidget* inner = panel->getPanel();

            const QList<QString> FaceRenderModes = { "Show textures", "Hide textures", "Hide faces" };
            const QList<QString> FaceRenderModesPrefValues = { Preferences::faceRenderModeTextured(), Preferences::faceRenderModeFlat(), Preferences::faceRenderModeSkip() };

            m_renderModeRadioGroup = new QButtonGroup(this);
            for (int i = 0; i < FaceRenderModes.length(); ++i) {
                const QString& label = FaceRenderModes.at(i);
                const QString& prefValue = FaceRenderModesPrefValues.at(i);

                auto* radio = new QRadioButton(label);
                radio->setObjectName(prefValue);
                m_renderModeRadioGroup->addButton(radio, i);
            }

            m_shadeFacesCheckBox = new QCheckBox(tr("Shade faces"));
            m_showFogCheckBox = new QCheckBox(tr("Use fog"));
            m_showEdgesCheckBox = new QCheckBox(tr("Show edges"));

            const QList<QString> EntityLinkModes = { "Show all entity links", "Show transitively selected entity links", "Show directly selected entity links", "Hide entity links" };
            const QList<QString> EntityLinkModesPrefValues = { Preferences::entityLinkModeAll(), Preferences::entityLinkModeTransitive(), Preferences::entityLinkModeDirect(), Preferences::entityLinkModeNone() };
            m_entityLinkRadioGroup = new QButtonGroup(this);
            for (int i = 0; i < EntityLinkModes.length(); ++i) {
                const QString& label = EntityLinkModes.at(i);
                const QString& prefValue = EntityLinkModesPrefValues.at(i);

                auto* radio = new QRadioButton(label);
                radio->setObjectName(prefValue);
                m_entityLinkRadioGroup->addButton(radio, i);
            }

            m_showSoftBoundsCheckBox = new QCheckBox(tr("Show soft bounds"));

            auto* restoreDefualtsButton = new QPushButton(tr("Restore Defaults"));
            makeEmphasized(restoreDefualtsButton);

            connect(m_shadeFacesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::shadeFacesChanged);
            connect(m_showFogCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showFogChanged);
            connect(m_showEdgesCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showEdgesChanged);

            connect(m_renderModeRadioGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
                &ViewEditor::faceRenderModeChanged);
            connect(m_entityLinkRadioGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
                &ViewEditor::entityLinkModeChanged);

            connect(m_showSoftBoundsCheckBox, &QAbstractButton::clicked, this, &ViewEditor::showSoftMapBoundsChanged);
            connect(restoreDefualtsButton, &QAbstractButton::clicked, this, &ViewEditor::restoreDefaultsClicked);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            for (auto* button : m_renderModeRadioGroup->buttons()) {
                layout->addWidget(button);
            }

            layout->addWidget(m_shadeFacesCheckBox);
            layout->addWidget(m_showFogCheckBox);
            layout->addWidget(m_showEdgesCheckBox);

            for (auto* button : m_entityLinkRadioGroup->buttons()) {
                layout->addWidget(button);
            }
            
            layout->addWidget(m_showSoftBoundsCheckBox);
            layout->addSpacing(LayoutConstants::MediumVMargin);
            layout->addWidget(restoreDefualtsButton, 0, Qt::AlignHCenter);

            inner->setLayout(layout);
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
            auto document = kdl::mem_lock(m_document);

            m_showEntityClassnamesCheckBox->setChecked(pref(Preferences::ShowEntityClassnames));
            m_showGroupBoundsCheckBox->setChecked(pref(Preferences::ShowGroupBounds));
            m_showBrushEntityBoundsCheckBox->setChecked(pref(Preferences::ShowBrushEntityBounds));
            m_showPointEntityBoundsCheckBox->setChecked(pref(Preferences::ShowPointEntityBounds));
            m_showPointEntitiesCheckBox->setChecked(pref(Preferences::ShowPointEntities));
            m_showPointEntityModelsCheckBox->setChecked(pref(Preferences::ShowPointEntityModels));
        }

        void ViewEditor::refreshBrushesPanel() {
            auto document = kdl::mem_lock(m_document);

            m_showBrushesCheckBox->setChecked(pref(Preferences::ShowBrushes));

            Model::EditorContext& editorContext = document->editorContext();
            const Model::TagType::Type hiddenTags = editorContext.hiddenTags();

            for (const auto& [tagType, checkBox] : m_tagCheckBoxes) {
                checkBox->setChecked((tagType & hiddenTags) == 0);
            }
        }

        void ViewEditor::refreshRendererPanel() {
            checkButtonInGroup(m_renderModeRadioGroup, pref(Preferences::FaceRenderMode), true);
            m_shadeFacesCheckBox->setChecked(pref(Preferences::ShadeFaces));
            m_showFogCheckBox->setChecked(pref(Preferences::ShowFog));
            m_showEdgesCheckBox->setChecked(pref(Preferences::ShowEdges));
            checkButtonInGroup(m_entityLinkRadioGroup, pref(Preferences::EntityLinkMode), true);
            m_showSoftBoundsCheckBox->setChecked(pref(Preferences::ShowSoftMapBounds));
        }

        void ViewEditor::showEntityClassnamesChanged(const bool checked) {
            setPref(Preferences::ShowEntityClassnames, checked);
        }

        void ViewEditor::showGroupBoundsChanged(const bool checked) {
            setPref(Preferences::ShowGroupBounds, checked);
        }

        void ViewEditor::showBrushEntityBoundsChanged(const bool checked) {
            setPref(Preferences::ShowBrushEntityBounds, checked);
        }

        void ViewEditor::showPointEntityBoundsChanged(const bool checked) {
            setPref(Preferences::ShowPointEntityBounds, checked);
        }

        void ViewEditor::showPointEntitiesChanged(const bool checked) {
            setPref(Preferences::ShowPointEntities, checked);
        }

        void ViewEditor::showPointEntityModelsChanged(const bool checked) {
            setPref(Preferences::ShowPointEntityModels, checked);
        }

        void ViewEditor::showBrushesChanged(const bool checked) {
            setPref(Preferences::ShowBrushes, checked);
        }

        void ViewEditor::showTagChanged(const bool checked, const Model::TagType::Type tagType) {
            auto document = kdl::mem_lock(m_document);
            auto& editorContext = document->editorContext();

            Model::TagType::Type hiddenTags = editorContext.hiddenTags();
            if (checked) {
                // Unhide tagType
                hiddenTags &= ~tagType;
            } else {
                // Hide tagType
                hiddenTags |= tagType;
            }

            editorContext.setHiddenTags(hiddenTags);
        }

        void ViewEditor::faceRenderModeChanged(const int id) {
            switch (id) {
                case 1:
                    setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeFlat());
                    break;
                case 2:
                    setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeSkip());
                    break;
                default:
                    setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeTextured());
                    break;
            }
        }

        void ViewEditor::shadeFacesChanged(const bool checked) {
            setPref(Preferences::ShadeFaces, checked);
        }

        void ViewEditor::showFogChanged(const bool checked) {
            setPref(Preferences::ShowFog, checked);
        }

        void ViewEditor::showEdgesChanged(const bool checked) {
            setPref(Preferences::ShowEdges, checked);
        }

        void ViewEditor::entityLinkModeChanged(const int id) {
            switch (id) {
                case 0:
                    setPref(Preferences::EntityLinkMode, Preferences::entityLinkModeAll());
                    break;
                case 1:
                    setPref(Preferences::EntityLinkMode, Preferences::entityLinkModeTransitive());
                    break;
                case 2:
                    setPref(Preferences::EntityLinkMode, Preferences::entityLinkModeDirect());
                    break;
                default:
                    setPref(Preferences::EntityLinkMode, Preferences::entityLinkModeNone());
                    break;
            }
        }

        void ViewEditor::showSoftMapBoundsChanged(const bool checked) {
            setPref(Preferences::ShowSoftMapBounds, checked);
        }

        void ViewEditor::restoreDefaultsClicked() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(Preferences::ShowEntityClassnames);
            prefs.resetToDefault(Preferences::ShowGroupBounds);
            prefs.resetToDefault(Preferences::ShowBrushEntityBounds);
            prefs.resetToDefault(Preferences::ShowPointEntityBounds);
            prefs.resetToDefault(Preferences::ShowPointEntityModels);
            prefs.resetToDefault(Preferences::FaceRenderMode);
            prefs.resetToDefault(Preferences::ShadeFaces);
            prefs.resetToDefault(Preferences::ShowFog);
            prefs.resetToDefault(Preferences::ShowEdges);
            prefs.resetToDefault(Preferences::ShowSoftMapBounds);
            prefs.resetToDefault(Preferences::ShowPointEntities);
            prefs.resetToDefault(Preferences::ShowBrushes);
            prefs.resetToDefault(Preferences::EntityLinkMode);
            prefs.saveChanges();
        }

        ViewPopupEditor::ViewPopupEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_button(nullptr),
        m_editor(nullptr) {
            m_button = new PopupButton(tr("View Options"));
            m_button->setToolTip(tr("Click to edit view settings"));

            auto* editorContainer = new BorderPanel();
            m_editor = new ViewEditor(document);

            auto* containerSizer = new QVBoxLayout();
            containerSizer->setContentsMargins(0, 0, 0, 0);
            containerSizer->addWidget(m_editor);
            editorContainer->setLayout(containerSizer);

            auto* popupSizer = new QVBoxLayout();
            popupSizer->setContentsMargins(0, 0, 0, 0);
            popupSizer->addWidget(editorContainer);
            m_button->GetPopupWindow()->setLayout(popupSizer);

            auto* sizer = new QHBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_button, Qt::AlignVCenter);

            setLayout(sizer);
        }
    }
}
