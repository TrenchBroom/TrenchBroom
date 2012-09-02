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

#ifndef __TrenchBroom__MapDocument__
#define __TrenchBroom__MapDocument__

#include "Utility/String.h"

#include <wx/docview.h>

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }
    
    namespace Model {
        class Brush;
        class EditStateManager;
        class Entity;
        class Face;
        class Map;
        class Octree;
        class Palette;
        class Picker;
        class TextureManager;
        
        class MapDocument : public wxDocument {
            DECLARE_DYNAMIC_CLASS(MapDocument)
        protected:
            Map* m_map;
            EditStateManager* m_editStateManager;
            Octree* m_octree;
            Picker* m_picker;
            Palette* m_palette;
            TextureManager* m_textureManager;
            
            virtual bool DoOpenDocument(const wxString& file);
            virtual bool DoSaveDocument(const wxString& file);
            
            void LoadTextureWad(const String& path);
            void UpdateFaceTextures();
            void Clear();
        public:
            MapDocument();
            virtual ~MapDocument();

            std::istream& LoadObject(std::istream& stream);
            std::ostream& SaveObject(std::ostream& stream);
            
            Map& Map() const;
            EditStateManager& EditStateManager() const;
            Picker& Picker() const;
            Utility::Console& Console() const;
            
            bool OnCreate(const wxString& path, long flags);
			bool OnNewDocument();
            bool OnOpenDocument(const wxString& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
