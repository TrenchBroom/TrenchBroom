/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityInspector__
#define __TrenchBroom__EntityInspector__

#include "Model/EntityTypes.h"
#include "View/EntityPropertyDataViewModel.h"

#include <wx/object.h>
#include <wx/panel.h>

class wxButton;
class wxDataViewCtrl;
class wxGLContext;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class DocumentViewHolder;
        class EntityBrowser;
        
        class EntityInspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;

            wxDataViewCtrl* m_propertyView;
            wxDataViewColumn* m_keyColumn;
            wxDataViewColumn* m_valueColumn;
            wxObjectDataPtr<EntityPropertyDataViewModel> m_propertyViewModel;
            wxButton* m_addPropertyButton;
            wxButton* m_removePropertiesButton;
            
            EntityBrowser* m_entityBrowser;
            
            wxWindow* createPropertyEditor(wxWindow* parent);
            wxWindow* createEntityBrowser(wxWindow* parent);
        public:
            EntityInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder);
            
            void updateProperties();
            void updateEntityBrowser();

            void OnAddPropertyPressed(wxCommandEvent& event);
            void OnRemovePropertiesPressed(wxCommandEvent& event);
            void OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event);
            void OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event);

            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__EntityInspector__) */
