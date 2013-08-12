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

#ifndef __TrenchBroom__EntityBrowser__
#define __TrenchBroom__EntityBrowser__

#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxToggleButton;
class wxChoice;
class wxGLContext;
class wxScrollBar;
class wxSearchCtrl;

namespace TrenchBroom {
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class EntityBrowserView;
        
        class EntityBrowser : public wxPanel {
        private:
            wxChoice* m_sortOrderChoice;
            wxToggleButton* m_groupButton;
            wxToggleButton* m_usedButton;
            wxSearchCtrl* m_filterBox;
            wxScrollBar* m_scrollBar;
            EntityBrowserView* m_view;
        public:
            EntityBrowser(wxWindow* parent, const wxWindowID windowId, Renderer::RenderResources& resources, MapDocumentPtr document);
            
            void reload();

            void OnSortOrderChanged(wxCommandEvent& event);
            void OnGroupButtonToggled(wxCommandEvent& event);
            void OnUsedButtonToggled(wxCommandEvent& event);
            void OnFilterPatternChanged(wxCommandEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityBrowser__) */
