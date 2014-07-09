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

#include "TextureCollectionEditorDropTarget.h"

#include "View/ViewUtils.h"
#include "View/wxUtils.h"

namespace TrenchBroom {
    namespace View {
        TextureCollectionEditorDropTarget::TextureCollectionEditorDropTarget(MapDocumentWPtr document, ControllerWPtr controller, wxWindow* parent) :
        wxFileDropTarget(),
        m_document(document),
        m_controller(controller),
        m_parent(parent) {
            SetDefaultAction(wxDragCopy);
        }

        bool TextureCollectionEditorDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
            return loadTextureCollections(m_document, m_controller, m_parent, filenames) > 0;
        }
    }
}
