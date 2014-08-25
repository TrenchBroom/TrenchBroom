/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__ModelFilterPopupEditor__
#define __TrenchBroom__ModelFilterPopupEditor__

#include "Model/BrushContentType.h"
#include "View/BorderPanel.h"
#include "View/ViewTypes.h"

#include "wx/panel.h"

#include <vector>

class wxCheckBox;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class PopupButton;
        
        class ModelFilterEditor : public BorderPanel {
        private:
            typedef std::vector<wxCheckBox*> CheckBoxList;
            
            MapDocumentWPtr m_document;
            
            wxCheckBox* m_showEntityClassnamesCheckBox;
            wxCheckBox* m_showEntityBoundsCheckBox;
            wxCheckBox* m_showPointEntitiesCheckBox;
            wxCheckBox* m_showPointEntityModelsCheckBox;
            
            wxCheckBox* m_showBrushesCheckBox;
            CheckBoxList m_brushContentTypeCheckBoxes;
        public:
            ModelFilterEditor(wxWindow* parent, MapDocumentWPtr document);
            ~ModelFilterEditor();
            
            void OnShowEntityClassnamesChanged(wxCommandEvent& event);
            void OnShowEntityBoundsChanged(wxCommandEvent& event);
            void OnShowPointEntitiesChanged(wxCommandEvent& event);
            void OnShowPointEntityModelsChanged(wxCommandEvent& event);
            void OnShowBrushesChanged(wxCommandEvent& event);
            void OnShowBrushContentTypeChanged(wxCommandEvent& event);
        private:
            void bindObservers();
            void unbindObservers();

            void documentWasNewedOrLoaded();
            void modelFilterDidChange();
            void renderConfigDidChange();
            
            void createGui();
            
            wxWindow* createEntitiesPanel();
            wxWindow* createBrushesPanel();
            void createBrushContentTypeFilter(wxWindow* parent);
            void createEmptyBrushContentTypeFilter(wxWindow* parent);
            void createBrushContentTypeFilter(wxWindow* parent, const Model::BrushContentType::List& contentTypes);
            
            void refreshGui();
            void refreshEntitiesPanel();
            void refreshBrushesPanel();
        };
        
        class ModelFilterPopupEditor : public wxPanel {
        private:
            PopupButton* m_button;
            ModelFilterEditor* m_editor;
        public:
            ModelFilterPopupEditor(wxWindow* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(__TrenchBroom__ModelFilterPopupEditor__) */
