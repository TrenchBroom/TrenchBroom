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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__TextureBrowser__
#define __TrenchBroom__TextureBrowser__

#include "StringUtils.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxChoice;
class wxToggleButton;
class wxSearchCtrl;
class wxScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class FaceTexture;
    }
    
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class TextureBrowserView;
        class TextureSelectedCommand;
        
        class TextureBrowser : public wxPanel {
        private:
            wxChoice* m_sortOrderChoice;
            wxToggleButton* m_groupButton;
            wxToggleButton* m_usedButton;
            wxSearchCtrl* m_filterBox;
            wxScrollBar* m_scrollBar;
            TextureBrowserView* m_view;
        public:
            TextureBrowser(wxWindow* parent, Renderer::RenderResources& resources, MapDocumentPtr document);
            ~TextureBrowser();
            
            void reload();
            
            Assets::FaceTexture* selectedTexture() const;
            void setSelectedTexture(Assets::FaceTexture* selectedTexture);
            
            void OnSortOrderChanged(wxCommandEvent& event);
            void OnGroupButtonToggled(wxCommandEvent& event);
            void OnUsedButtonToggled(wxCommandEvent& event);
            void OnFilterPatternChanged(wxCommandEvent& event);
            void OnTextureSelected(TextureSelectedCommand& event);
        private:
            void preferenceDidChange(const String& name);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureBrowser__) */
