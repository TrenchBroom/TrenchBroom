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

#ifndef __TrenchBroom__ToolBoxDropTarget__
#define __TrenchBroom__ToolBoxDropTarget__

#include "StringUtils.h"

#include <wx/dnd.h>

namespace TrenchBroom {
    namespace View {
        class ToolBox;
        
        class ToolBoxDropTarget : public wxTextDropTarget {
        private:
            ToolBox& m_toolBox;
        public:
            ToolBoxDropTarget(ToolBox& toolBox);
            
            wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
            wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
            void OnLeave();
            bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
        private:
            String getDragText() const;
        };
    }
}

#endif /* defined(__TrenchBroom__ToolBoxDropTarget__) */
