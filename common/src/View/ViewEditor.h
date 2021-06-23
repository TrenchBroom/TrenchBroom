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

#pragma once

#include "NotifierConnection.h"
#include "Model/TagType.h"

#include <memory>
#include <vector>


#include <QWidget>

class QCheckBox;
class QWidget;
class QButtonGroup;

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class EntityDefinitionManager;
    }

    namespace IO {
        class Path;
    }

    namespace Model {
        class EditorContext;
        class SmartTag;
    }

    namespace View {
        class MapDocument;
        class PopupButton;

        class EntityDefinitionCheckBoxList : public QWidget {
            Q_OBJECT
        private:
            using CheckBoxList = std::vector<QCheckBox*>;

            Assets::EntityDefinitionManager& m_entityDefinitionManager;
            Model::EditorContext& m_editorContext;

            CheckBoxList m_groupCheckBoxes;
            CheckBoxList m_defCheckBoxes;
        public:
            EntityDefinitionCheckBoxList(Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext, QWidget* parent = nullptr);

            void refresh();
        private slots:
            void groupCheckBoxChanged(size_t groupIndex, bool checked);
            void defCheckBoxChanged(const Assets::EntityDefinition* definition, bool checked);
            void showAllClicked();
            void hideAllClicked();
        private:
            void hideAll(bool hidden);
            void createGui();
        };

        class ViewEditor : public QWidget {
            Q_OBJECT
        private:
            using CheckBoxList = std::vector<QCheckBox*>;

            std::weak_ptr<MapDocument> m_document;

            QCheckBox* m_showEntityClassnamesCheckBox;

            QCheckBox* m_showGroupBoundsCheckBox;
            QCheckBox* m_showBrushEntityBoundsCheckBox;
            QCheckBox* m_showPointEntityBoundsCheckBox;

            QCheckBox* m_showPointEntitiesCheckBox;
            QCheckBox* m_showPointEntityModelsCheckBox;

            EntityDefinitionCheckBoxList* m_entityDefinitionCheckBoxList;

            QCheckBox* m_showBrushesCheckBox;
            std::vector<std::pair<Model::TagType::Type, QCheckBox*>> m_tagCheckBoxes;

            QButtonGroup* m_renderModeRadioGroup;
            QCheckBox* m_shadeFacesCheckBox;
            QCheckBox* m_showFogCheckBox;
            QCheckBox* m_showEdgesCheckBox;

            QButtonGroup* m_entityLinkRadioGroup;

            QCheckBox* m_showSoftBoundsCheckBox;

            NotifierConnection m_notifierConnection;
        public:
            explicit ViewEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void connectObservers();

            void documentWasNewedOrLoaded(MapDocument* document);
            void editorContextDidChange();
            void entityDefinitionsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void createGui();

            QWidget* createEntityDefinitionsPanel(QWidget* parent);
            QWidget* createEntitiesPanel(QWidget* parent);
            QWidget* createBrushesPanel(QWidget* parent);
            void createTagFilter(QWidget* parent);
            void createEmptyTagFilter(QWidget* parent);
            void createTagFilter(QWidget* parent, const std::vector<Model::SmartTag>& tags);

            QWidget* createRendererPanel(QWidget* parent);

            void refreshGui();
            void refreshEntityDefinitionsPanel();
            void refreshEntitiesPanel();
            void refreshBrushesPanel();
            void refreshRendererPanel();

            void showEntityClassnamesChanged(bool checked);
            void showGroupBoundsChanged(bool checked);
            void showBrushEntityBoundsChanged(bool checked);
            void showPointEntityBoundsChanged(bool checked);
            void showPointEntitiesChanged(bool checked);
            void showPointEntityModelsChanged(bool checked);
            void showBrushesChanged(bool checked);
            void showTagChanged(bool checked, Model::TagType::Type tagType);
            void faceRenderModeChanged(int id);
            void shadeFacesChanged(bool checked);
            void showFogChanged(bool checked);
            void showEdgesChanged(bool checked);
            void entityLinkModeChanged(int id);
            void showSoftMapBoundsChanged(bool checked);
            void restoreDefaultsClicked();
        };

        class ViewPopupEditor : public QWidget {
            Q_OBJECT
        private:
            PopupButton* m_button;
            ViewEditor* m_editor;
        public:
            explicit ViewPopupEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        };
    }
}

