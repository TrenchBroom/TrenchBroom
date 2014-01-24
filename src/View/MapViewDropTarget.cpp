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

#include "MapViewDropTarget.h"
#include "View/GenericDropSource.h"
#include "View/MapView.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapViewDropTarget::MapViewDropTarget(MapView* view) :
        wxTextDropTarget(),
        m_view(view) {}
        
        wxDragResult MapViewDropTarget::OnEnter(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_view->dragEnter(getDragText(), x, y))
                return wxTextDropTarget::OnEnter(x, y, wxDragCopy);
            else
                return wxTextDropTarget::OnEnter(x, y, wxDragNone);
        }
        
        wxDragResult MapViewDropTarget::OnDragOver(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_view->dragMove(getDragText(), x, y))
                return wxTextDropTarget::OnDragOver(x, y, wxDragCopy);
            else
                return wxTextDropTarget::OnDragOver(x, y, wxDragNone);
        }
        
        void MapViewDropTarget::OnLeave() {
            wxTextDropTarget::OnLeave();
            m_view->dragLeave();
        }
        
        bool MapViewDropTarget::OnDropText(const wxCoord x, const wxCoord y, const wxString& data) {
            return m_view->dragDrop(getDragText(), x, y);
        }

        String MapViewDropTarget::getDragText() const {
            assert(CurrentDropSource != NULL);
            const wxTextDataObject* dataObject = static_cast<wxTextDataObject*>(CurrentDropSource->GetDataObject());
            return dataObject->GetText().ToStdString();
        }
    }
}
