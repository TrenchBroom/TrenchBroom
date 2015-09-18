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

#ifndef TrenchBroom_TextureCollectionEditor
#define TrenchBroom_TextureCollectionEditor

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
            
            wxListBox* m_collections;
        public:
            TextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document);
            ~TextureCollectionEditor();
            
            void OnAddTextureCollectionsClicked(wxCommandEvent& event);
            void OnRemoveTextureCollectionsClicked(wxCommandEvent& event);
            void OnMoveTextureCollectionUpClicked(wxCommandEvent& event);
            void OnMoveTextureCollectionDownClicked(wxCommandEvent& event);
            void OnUpdateRemoveButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event);
            void OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event);
        private:
            void createGui();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void textureCollectionsDidChange();
            void preferenceDidChange(const IO::Path& path);
            
            void updateControls();
        };
    }
}

#endif /* defined(TrenchBroom_TextureCollectionEditor) */
