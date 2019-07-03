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

#ifndef TrenchBroom_ViewEditor
#define TrenchBroom_ViewEditor

#include "View/ViewTypes.h"

#include <QWidget>

#include <list>
#include <vector>

class QCheckBox;
class QWidget;
class QButtonGroup;

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionManager;
        class EntityDefinition;
    }

    namespace Model {
        class EditorContext;
        class SmartTag;
    }

    namespace View {
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
            EntityDefinitionCheckBoxList(QWidget* parent, Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext);

            void refresh();

            // FIXME: turn into slots, rename to camelCase, remove "on" prefix
            void OnGroupCheckBoxChanged(size_t groupIndex, bool checked);
            void OnDefCheckBoxChanged(const Assets::EntityDefinition* definition, bool checked);
            void OnShowAllClicked();
            void OnHideAllClicked();
        private:
            void hideAll(bool hidden);
            void createGui();
        };

        class ViewEditor : public QWidget {
            Q_OBJECT
        private:
            using CheckBoxList = std::vector<QCheckBox*>;

            MapDocumentWPtr m_document;

            QCheckBox* m_showEntityClassnamesCheckBox;

            QCheckBox* m_showGroupBoundsCheckBox;
            QCheckBox* m_showBrushEntityBoundsCheckBox;
            QCheckBox* m_showPointEntityBoundsCheckBox;

            QCheckBox* m_showPointEntitiesCheckBox;
            QCheckBox* m_showPointEntityModelsCheckBox;

            EntityDefinitionCheckBoxList* m_entityDefinitionCheckBoxList;

            QCheckBox* m_showBrushesCheckBox;
            CheckBoxList m_tagCheckBoxes;

            QButtonGroup* m_renderModeRadioGroup;
            QCheckBox* m_shadeFacesCheckBox;
            QCheckBox* m_showFogCheckBox;
            QCheckBox* m_showEdgesCheckBox;

            QButtonGroup* m_entityLinkRadioGroup;
        public:
            ViewEditor(QWidget* parent, MapDocumentWPtr document);
            ~ViewEditor() override;

            // FIXME: turn into slots, rename to camelCase, remove "on" prefix
            void OnShowEntityClassnamesChanged(bool checked);
            void OnShowGroupBoundsChanged(bool checked);
            void OnShowBrushEntityBoundsChanged(bool checked);
            void OnShowPointEntityBoundsChanged(bool checked);
            void OnShowPointEntitiesChanged(bool checked);
            void OnShowPointEntityModelsChanged(bool checked);
            void OnShowBrushesChanged(bool checked);
            void OnShowTagChanged(bool checked);
            void OnFaceRenderModeChanged(int id);
            void OnShadeFacesChanged(bool checked);
            void OnShowFogChanged(bool checked);
            void OnShowEdgesChanged(bool checked);
            void OnEntityLinkModeChanged(int id);
        private:
            void bindObservers();
            void unbindObservers();

            void documentWasNewedOrLoaded(MapDocument* document);
            void editorContextDidChange();
            void mapViewConfigDidChange();
            void entityDefinitionsDidChange();

            void createGui();

            QWidget* createEntityDefinitionsPanel(QWidget* parent);
            QWidget* createEntitiesPanel(QWidget* parent);
            QWidget* createBrushesPanel(QWidget* parent);
            void createTagFilter(QWidget* parent);
            void createEmptyTagFilter(QWidget* parent);
            void createTagFilter(QWidget* parent, const std::list<Model::SmartTag>& tags);

            QWidget* createRendererPanel(QWidget* parent);

            void refreshGui();
            void refreshEntityDefinitionsPanel();
            void refreshEntitiesPanel();
            void refreshBrushesPanel();
            void refreshRendererPanel();
        };

        class ViewPopupEditor : public QWidget {
            Q_OBJECT
        private:
            PopupButton* m_button;
            ViewEditor* m_editor;
        public:
            ViewPopupEditor(QWidget* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_ViewEditor) */
