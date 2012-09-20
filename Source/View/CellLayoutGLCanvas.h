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

#ifndef TrenchBroom_CellLayoutGLCanvas_h
#define TrenchBroom_CellLayoutGLCanvas_h

#include <wx/glcanvas.h>

#include "Renderer/Text/FontDescriptor.h"
#include "View/CellLayout.h"

namespace TrenchBroom {
    namespace View {
        template <typename CellData, typename GroupData>
        class CellLayoutGLCanvas : public wxGLCanvas {
        protected:
            typedef CellLayout<CellData, GroupData> Layout;
            Layout m_layout;
            typename Layout::Group::Row::Cell* m_selectedCell;
            
            Renderer::Text::FontDescriptor m_font;
            
            virtual void reloadLayout() {
                m_layout.clear();
                doReloadLayout();
                
                // update actual height and scrollbars here
            }
            
            virtual void doReloadLayout() = 0;
        };
    }
}

#endif
