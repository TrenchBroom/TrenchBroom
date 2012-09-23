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

#ifndef __TrenchBroom__TextureBrowser__
#define __TrenchBroom__TextureBrowser__

#include <wx/panel.h>

class wxToggleButton;
class wxChoice;
class wxGLContext;
class wxSearchCtrl;

namespace TrenchBroom {
    namespace Model {
        class TextureManager;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class TextureBrowserCanvas;
        
        class TextureBrowser : public wxPanel {
        protected:
            wxChoice* m_sortOrderChoice;
            wxToggleButton* m_groupButton;
            wxToggleButton* m_usedButton;
            wxSearchCtrl* m_filterBox;
            TextureBrowserCanvas* m_canvas;
            wxScrollBar* m_scrollBar;
        public:
            TextureBrowser(wxWindow* parent, wxGLContext* sharedContext, Utility::Console& console, Model::TextureManager& textureManager);
            
            void reload();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureBrowser__) */
