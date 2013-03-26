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

#ifndef __TrenchBroom__ViewInspector__
#define __TrenchBroom__ViewInspector__

#include <wx/event.h>
#include <wx/panel.h>

class wxCheckBox;
class wxChoice;
class wxSearchCtrl;

namespace TrenchBroom {
    namespace View {
        class DocumentViewHolder;
        class EditorView;
        class ViewOptions;
        
        class ViewInspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            wxSearchCtrl* m_searchBox;
            wxCheckBox* m_toggleEntities;
            wxCheckBox* m_toggleEntityModels;
            wxCheckBox* m_toggleEntityBounds;
            wxCheckBox* m_toggleEntityClassnames;
            wxCheckBox* m_toggleBrushes;
            wxCheckBox* m_toggleClipBrushes;
            wxCheckBox* m_toggleSkipBrushes;
            wxChoice* m_faceRenderModeChoice;
            wxCheckBox* m_toggleRenderEdges;
            wxCheckBox* m_toggleFaceShading;
            // wxCheckBox* m_toggleFog;
            wxChoice* m_linkDisplayModeChoice;
            
            void updateControls();
            
            wxWindow* createFilterBox();
            wxWindow* createRenderModeSelector();
        public:
            ViewInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder);
            
            void OnFilterPatternChanged(wxCommandEvent& event);
            void OnFilterOptionChanged(wxCommandEvent& event);
            void OnRenderFaceModeSelected(wxCommandEvent& event);
            void OnRenderEdgesChanged(wxCommandEvent& event);
            void OnFaceShadingChanged(wxCommandEvent& event);
            // void OnFogChanged(wxCommandEvent& event);
            void OnLinkDisplayModeSelected(wxCommandEvent& event);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__ViewInspector__) */
