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

#include "TextureCollectionEditor.h"

#include "Model/Game.h"
#include "View/DirectoryTextureCollectionEditor.h"
#include "View/FileTextureCollectionEditor.h"
#include "View/MapDocument.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TextureCollectionEditor::TextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            MapDocumentSPtr doc = lock(m_document);
            doc->documentWasNewedNotifier.addObserver(this, &TextureCollectionEditor::documentWasNewed);
            doc->documentWasLoadedNotifier.addObserver(this, &TextureCollectionEditor::documentWasLoaded);
        }
        
        TextureCollectionEditor::~TextureCollectionEditor() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasLoaded);
            }
        }

        void TextureCollectionEditor::documentWasNewed(MapDocument* document) {
            DestroyChildren();
            createGui();
        }
        
        void TextureCollectionEditor::documentWasLoaded(MapDocument* document) {
            DestroyChildren();
            createGui();
        }
        
        void TextureCollectionEditor::createGui() {
            wxWindow* collectionEditor = NULL;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::Game::TexturePackageType type = document->game()->texturePackageType();
            switch (type) {
                case Model::Game::TP_File:
                    collectionEditor = new FileTextureCollectionEditor(this, m_document);
                    break;
                case Model::Game::TP_Directory:
                    collectionEditor = new DirectoryTextureCollectionEditor(this, m_document);
                    break;
            }
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(collectionEditor, wxSizerFlags().Expand().Proportion(1));
            
            SetSizer(sizer);
        }
    }
}
