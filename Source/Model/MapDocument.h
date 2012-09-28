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
        class Grid;
        class ProgressIndicator;
    }
    
    namespace Model {
        class Brush;
        class EditStateManager;
        class Entity;
        class EntityDefinitionManager;
        class Face;
        class Map;
        class Octree;
        class Palette;
        class Picker;
        class Texture;
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
            EntityDefinitionManager* m_definitionManager;
            Utility::Grid* m_grid;
            StringList m_mods;
            Model::Texture* m_mruTexture;
            String m_mruTextureName;
            
            virtual bool DoOpenDocument(const wxString& file);
            virtual bool DoSaveDocument(const wxString& file);
            
            void updateEntityDefinitions();
            void clear();

            void loadPalette();
            void loadMap(std::istream& stream, Utility::ProgressIndicator& progressIndicator);
            void loadTextures(Utility::ProgressIndicator& progressIndicator);
            void loadEntityDefinitions(Utility::ProgressIndicator& progressIndicator);
        public:
            MapDocument();
            virtual ~MapDocument();

            std::istream& LoadObject(std::istream& stream);
            std::ostream& SaveObject(std::ostream& stream);
            
            Map& map() const;
            EntityDefinitionManager& definitionManager() const;
            EditStateManager& editStateManager() const;
            TextureManager& textureManager() const;
            Picker& picker() const;
            Utility::Grid& grid() const;
            Utility::Console& console() const;
            const StringList& mods() const;
            const Palette& palette() const;
            
            Model::Texture* mruTexture() const;
            void setMruTexture(Model::Texture* texture);
            
            void updateAfterTextureManagerChanged();
            void loadTextureWad(const String& path);
            void loadTextureWad(const String& path, size_t index);

            bool OnCreate(const wxString& path, long flags);
			bool OnNewDocument();
            bool OnOpenDocument(const wxString& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
