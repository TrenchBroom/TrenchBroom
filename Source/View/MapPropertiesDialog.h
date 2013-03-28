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

#ifndef __TrenchBroom__MapPropertiesDialog__
#define __TrenchBroom__MapPropertiesDialog__

#include "Utility/String.h"

#include <wx/dialog.h>
#include <wx/vlbox.h>

class wxButton;
class wxCheckBox;
class wxChoice;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
        class TextureManager;
    }
    
    namespace View {
        class WadListBox : public wxVListBox {
        private:
            Model::TextureManager& m_textureManager;
        public:
            WadListBox(wxWindow* parent, wxWindowID windowId, Model::TextureManager& textureManager);
            
            void OnDrawItem (wxDC &dc, const wxRect &rect, size_t n) const;
            void OnDrawBackground (wxDC &dc, const wxRect &rect, size_t n) const;
            wxCoord OnMeasureItem (size_t n) const;
            
            void GetSelections(wxArrayInt& selection) const;
        };
        
        class MapPropertiesDialog : public wxDialog {
        protected:
            Model::MapDocument& m_document;
            
            wxChoice* m_modChoice;
            wxChoice* m_defChoice;
            wxCheckBox* m_intFacePointsCheckBox;
            WadListBox* m_wadList;
            wxButton* m_addWadButton;
            wxButton* m_removeWadsButton;
            wxButton* m_moveWadUpButton;
            wxButton* m_moveWadDownButton;
            
            void populateDefChoice(const String& def);
            void populateModChoice(const String& mod);
            void populateWadList();
            
            void init();
        public:
            MapPropertiesDialog(wxWindow* parent, Model::MapDocument& document);
            
            void EndModal(int retCode);
            
            void OnDefChoiceSelected(wxCommandEvent& event);
            void OnModChoiceSelected(wxCommandEvent& event);
            void OnIntFacePointsCheckBoxClicked(wxCommandEvent& event);

            void OnAddWadClicked(wxCommandEvent& event);
            void OnRemoveWadsClicked(wxCommandEvent& event);
            void OnMoveWadUpClicked(wxCommandEvent& event);
            void OnMoveWadDownClicked(wxCommandEvent& event);
            void OnUpdateWadButtons(wxUpdateUIEvent& event);
            void OnCloseClicked(wxCommandEvent& event);
            void OnFileExit(wxCommandEvent& event);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__MapPropertiesDialog__) */
