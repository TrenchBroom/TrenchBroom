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

#include <wx/panel.h>

#include <list>
#include <vector>

class wxCheckBox;
class wxWindow;

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionManager;
    }

    namespace Model {
        class EditorContext;
        class SmartTag;
    }

    namespace View {
        class PopupButton;
        class RadioGroup;

        class EntityDefinitionCheckBoxList : public wxPanel {
        private:
            using CheckBoxList = std::vector<wxCheckBox*>;

            Assets::EntityDefinitionManager& m_entityDefinitionManager;
            Model::EditorContext& m_editorContext;

            CheckBoxList m_groupCheckBoxes;
            CheckBoxList m_defCheckBoxes;
        public:
            EntityDefinitionCheckBoxList(wxWindow* parent, Assets::EntityDefinitionManager& entityDefinitionManager, Model::EditorContext& editorContext);

            void refresh();

            void OnGroupCheckBoxChanged(wxCommandEvent& event);
            void OnDefCheckBoxChanged(wxCommandEvent& event);
            void OnShowAllClicked(wxCommandEvent& event);
            void OnHideAllClicked(wxCommandEvent& event);
        private:
            void hideAll(bool hidden);
            void createGui();
        };

        class ViewEditor : public wxPanel {
        private:
            using CheckBoxList = std::vector<wxCheckBox*>;

            MapDocumentWPtr m_document;

            wxCheckBox* m_showEntityClassnamesCheckBox;

            wxCheckBox* m_showGroupBoundsCheckBox;
            wxCheckBox* m_showBrushEntityBoundsCheckBox;
            wxCheckBox* m_showPointEntityBoundsCheckBox;

            wxCheckBox* m_showPointEntitiesCheckBox;
            wxCheckBox* m_showPointEntityModelsCheckBox;

            EntityDefinitionCheckBoxList* m_entityDefinitionCheckBoxList;

            wxCheckBox* m_showBrushesCheckBox;
            CheckBoxList m_tagCheckBoxes;

            RadioGroup* m_renderModeRadioGroup;
            wxCheckBox* m_shadeFacesCheckBox;
            wxCheckBox* m_showFogCheckBox;
            wxCheckBox* m_showEdgesCheckBox;

            RadioGroup* m_entityLinkRadioGroup;
        public:
            ViewEditor(wxWindow* parent, MapDocumentWPtr document);
            ~ViewEditor() override;

            void OnShowEntityClassnamesChanged(wxCommandEvent& event);
            void OnShowGroupBoundsChanged(wxCommandEvent& event);
            void OnShowBrushEntityBoundsChanged(wxCommandEvent& event);
            void OnShowPointEntityBoundsChanged(wxCommandEvent& event);
            void OnShowPointEntitiesChanged(wxCommandEvent& event);
            void OnShowPointEntityModelsChanged(wxCommandEvent& event);
            void OnShowBrushesChanged(wxCommandEvent& event);
            void OnShowTagChanged(wxCommandEvent& event);
            void OnFaceRenderModeChanged(wxCommandEvent& event);
            void OnShadeFacesChanged(wxCommandEvent& event);
            void OnShowFogChanged(wxCommandEvent& event);
            void OnShowEdgesChanged(wxCommandEvent& event);
            void OnEntityLinkModeChanged(wxCommandEvent& event);
        private:
            void bindObservers();
            void unbindObservers();

            void documentWasNewedOrLoaded(MapDocument* document);
            void editorContextDidChange();
            void mapViewConfigDidChange();
            void entityDefinitionsDidChange();

            void createGui();

            wxWindow* createEntityDefinitionsPanel(wxWindow* parent);
            wxWindow* createEntitiesPanel(wxWindow* parent);
            wxWindow* createBrushesPanel(wxWindow* parent);
            void createTagFilter(wxWindow* parent);
            void createEmptyTagFilter(wxWindow* parent);
            void createTagFilter(wxWindow* parent, const std::list<Model::SmartTag>& tags);

            wxWindow* createRendererPanel(wxWindow* parent);

            void refreshGui();
            void refreshEntityDefinitionsPanel();
            void refreshEntitiesPanel();
            void refreshBrushesPanel();
            void refreshRendererPanel();
        };

        class ViewPopupEditor : public wxPanel {
        private:
            PopupButton* m_button;
            ViewEditor* m_editor;
        public:
            ViewPopupEditor(wxWindow* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_ViewEditor) */
