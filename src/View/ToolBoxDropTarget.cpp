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

#include "ToolBoxDropTarget.h"
#include "View/DragAndDrop.h"
#include "View/ToolBox.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolBoxDropTarget::ToolBoxDropTarget(ToolBox& toolBox) :
        wxTextDropTarget(),
        m_toolBox(toolBox) {}
        
        wxDragResult ToolBoxDropTarget::OnEnter(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_toolBox.dragEnter(x, y, getDragText()))
                return wxTextDropTarget::OnEnter(x, y, wxDragCopy);
            return wxTextDropTarget::OnEnter(x, y, wxDragNone);
        }
        
        wxDragResult ToolBoxDropTarget::OnDragOver(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_toolBox.dragMove(x, y, getDragText()))
                return wxTextDropTarget::OnDragOver(x, y, wxDragCopy);
            return wxTextDropTarget::OnDragOver(x, y, wxDragNone);
        }
        
        void ToolBoxDropTarget::OnLeave() {
            m_toolBox.dragLeave();
            wxTextDropTarget::OnLeave();
        }
        
        bool ToolBoxDropTarget::OnDropText(const wxCoord x, const wxCoord y, const wxString& data) {
            return m_toolBox.dragDrop(x, y, data.ToStdString());
        }

        String ToolBoxDropTarget::getDragText() const {
            wxDropSource* currentDropSource = DropSource::getCurrentDropSource();
            assert(currentDropSource != NULL);
            const wxTextDataObject* dataObject = static_cast<wxTextDataObject*>(currentDropSource->GetDataObject());
            return dataObject->GetText().ToStdString();
        }
    }
}
