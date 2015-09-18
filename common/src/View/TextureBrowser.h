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

#ifndef TrenchBroom_TextureBrowser
#define TrenchBroom_TextureBrowser

#include "StringUtils.h"
#include "Assets/TextureManager.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxChoice;
class wxToggleButton;
class wxSearchCtrl;
class wxScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace IO {
        class Path;
    }
    
    namespace View {
        class GLContextManager;
        class TextureBrowserView;
        class TextureSelectedCommand;
        
        class TextureBrowser : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            wxChoice* m_sortOrderChoice;
            wxToggleButton* m_groupButton;
            wxToggleButton* m_usedButton;
            wxSearchCtrl* m_filterBox;
            wxScrollBar* m_scrollBar;
            TextureBrowserView* m_view;
        public:
            TextureBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            ~TextureBrowser();
            
            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);
            
            void setSortOrder(Assets::TextureManager::SortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const String& filterText);
            
            void OnSortOrderChanged(wxCommandEvent& event);
            void OnGroupButtonToggled(wxCommandEvent& event);
            void OnUsedButtonToggled(wxCommandEvent& event);
            void OnFilterPatternChanged(wxCommandEvent& event);
            void OnTextureSelected(TextureSelectedCommand& event);
        private:
            void createGui(GLContextManager& contextManager);
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void textureCollectionsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void reload();
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowser) */
