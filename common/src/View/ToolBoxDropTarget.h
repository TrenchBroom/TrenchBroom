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

#ifndef TrenchBroom_ToolBoxDropTarget
#define TrenchBroom_ToolBoxDropTarget

#include "StringUtils.h"

#include <wx/dnd.h>

class QWidget;

namespace TrenchBroom {
    namespace View {
        class ToolBoxConnector;
        
        class ToolBoxDropTarget : public wxTextDropTarget {
        private:
            QWidget* m_window;
            ToolBoxConnector* m_toolBoxConnector;
        public:
            ToolBoxDropTarget(QWidget* window, ToolBoxConnector* toolBoxConnector);
            
            wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def) override;
            wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
            void OnLeave() override;
            bool OnDropText(wxCoord x, wxCoord y, const QString& data) override;
        private:
            String getDragText() const;
        };
    }
}

#endif /* defined(TrenchBroom_ToolBoxDropTarget) */
