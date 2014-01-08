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

#ifndef __TrenchBroom__TextureCollectionEditor__
#define __TrenchBroom__TextureCollectionEditor__

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxBitmapButton;
class wxListBox;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class TextureCollectionEditor : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            wxListBox* m_collections;
            wxBitmapButton* m_addTextureCollectionsButton;
            wxBitmapButton* m_removeTextureCollectionsButton;
            wxBitmapButton* m_moveTextureCollectionUpButton;
            wxBitmapButton* m_moveTextureCollectionDownButton;
        public:
            TextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller);
            ~TextureCollectionEditor();
            
            void OnAddTextureCollectionsClicked(wxCommandEvent& event);
            void OnRemoveTextureCollectionsClicked(wxCommandEvent& event);
            void OnMoveTextureCollectionUpClicked(wxCommandEvent& event);
            void OnMoveTextureCollectionDownClicked(wxCommandEvent& event);
            void OnUpdateButtonUI(wxUpdateUIEvent& event);
        private:
            void createGui();
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void textureCollectionsDidChange();
            void preferenceDidChange(const IO::Path& path);
            
            void updateControls();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCollectionEditor__) */
