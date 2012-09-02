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

#ifndef __TrenchBroom__EditorFrame__
#define __TrenchBroom__EditorFrame__

#include <wx/frame.h>

class wxDocManager;
class wxTextCtrl;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class EditorView;
        class MapGLCanvas;
        
        class EditorFrame : public wxFrame {
        protected:
            Utility::Console* m_console;
            MapGLCanvas* m_mapCanvas;
            
            void CreateGui(Model::MapDocument& document, EditorView& view);
            void CreateMenuBar(Model::MapDocument& document);
        public:
            EditorFrame(Model::MapDocument& document, EditorView& view);
            ~EditorFrame();

            inline Utility::Console& Console() const {
                return *m_console;
            }
            
            inline MapGLCanvas& MapCanvas() const {
                return *m_mapCanvas;
            }
        };
    }
}


#endif /* defined(__TrenchBroom__EditorFrame__) */
