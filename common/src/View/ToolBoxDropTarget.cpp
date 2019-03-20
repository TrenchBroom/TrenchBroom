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

#include "ToolBoxDropTarget.h"
#include "View/DragAndDrop.h"
#include "View/ToolBoxConnector.h"

#include <cassert>

#include <wx/window.h>

namespace TrenchBroom {
    namespace View {
        ToolBoxDropTarget::ToolBoxDropTarget(QWidget* window, ToolBoxConnector* toolBoxConnector) :
        wxTextDropTarget(),
        m_window(window),
        m_toolBoxConnector(toolBoxConnector) {
            ensure(m_window != nullptr, "window is null");
            ensure(m_toolBoxConnector != nullptr, "toolBoxConnector is null");
        }

        wxDragResult ToolBoxDropTarget::OnEnter(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_toolBoxConnector->dragEnter(x, y, getDragText())) {
                m_window->Refresh();
                return wxTextDropTarget::OnEnter(x, y, wxDragCopy);
            }
            return wxTextDropTarget::OnEnter(x, y, wxDragNone);
        }

        wxDragResult ToolBoxDropTarget::OnDragOver(const wxCoord x, const wxCoord y, const wxDragResult def) {
            if (m_toolBoxConnector->dragMove(x, y, getDragText())) {
                m_window->Refresh();
                return wxTextDropTarget::OnDragOver(x, y, wxDragCopy);
            }
            return wxTextDropTarget::OnDragOver(x, y, wxDragNone);
        }

        void ToolBoxDropTarget::OnLeave() {
            m_toolBoxConnector->dragLeave();
            m_window->Refresh();
            wxTextDropTarget::OnLeave();
        }

        bool ToolBoxDropTarget::OnDropText(const wxCoord x, const wxCoord y, const QString& data) {
            const auto result = m_toolBoxConnector->dragDrop(x, y, data.ToStdString());
            m_window->SetFocus();
            m_window->Refresh();
            return result;
        }

        String ToolBoxDropTarget::getDragText() const {
            wxDropSource* currentDropSource = DropSource::getCurrentDropSource();
            ensure(currentDropSource != nullptr, "currentDropSource is null");
            const wxTextDataObject* dataObject = static_cast<wxTextDataObject*>(currentDropSource->GetDataObject());
            return dataObject->GetText().ToStdString();
        }
    }
}
