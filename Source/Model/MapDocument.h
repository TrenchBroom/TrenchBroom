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
        class Map;
        class Palette;
        class TextureManager;
        
        class MapDocument : public wxDocument {
            DECLARE_DYNAMIC_CLASS(MapDocument)
        protected:
            Map* m_map;
            Palette* m_palette;
            TextureManager* m_textureManager;
            
            virtual bool DoOpenDocument(const wxString& file);
            virtual bool DoSaveDocument(const wxString& file);
            
            void LoadTextureWad(const String& path);
            void UpdateFaceTextures();
        public:
            MapDocument();
            virtual ~MapDocument();

            std::istream& LoadObject(std::istream& stream);
            std::ostream& SaveObject(std::ostream& stream);
            
            Model::Map& Map() const;
            Utility::Console& Console() const;
            
            bool OnCreate(const wxString& path, long flags);
			bool OnNewDocument();
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
