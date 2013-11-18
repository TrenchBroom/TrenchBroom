/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Game__
#define __TrenchBroom__Game__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/GameFileSystem.h"
#include "Model/GameConfig.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace IO {
        class MapWriter;
    }
    
    namespace Model {
        class Game {
        public:
            virtual ~Game();

            const String& gameName() const;
            bool isGamePathPreference(const IO::Path& prefPath) const;
            
            void setGamePath(const IO::Path& gamePath);
            void setAdditionalSearchPaths(const IO::Path::List& searchPaths);
            
            Map* newMap(MapFormat::Type format) const;
            Map* loadMap(const BBox3& worldBounds, const IO::Path& path) const;
            Model::EntityList parseEntities(const BBox3& worldBounds, const String& str) const;
            Model::BrushList parseBrushes(const BBox3& worldBounds, const String& str) const;
            Model::BrushFaceList parseFaces(const BBox3& worldBounds, const String& str) const;
            
            void writeMap(Map& map, const IO::Path& path) const;
            void writeObjectsToStream(MapFormat::Type format, const Model::ObjectList& objects, std::ostream& stream) const;
            void writeFacesToStream(MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) const;
            
            IO::Path::List findBuiltinTextureCollections() const;
            IO::Path::List extractTexturePaths(const Map* map) const;
            Assets::TextureCollection* loadTextureCollection(const IO::Path& path) const;
            
            Assets::EntityDefinitionList loadEntityDefinitions(const IO::Path& path) const;
            IO::Path defaultEntityDefinitionFile() const;
            IO::Path::List allEntityDefinitionFiles() const;
            IO::Path extractEntityDefinitionFile(const Map* map) const;
            Assets::EntityModel* loadModel(const IO::Path& path) const;
            
            StringList availableMods() const;
            StringList extractEnabledMods(const Map* map) const;
        private:
            virtual const String& doGameName() const = 0;
            virtual void doSetGamePath(const IO::Path& gamePath) = 0;
            virtual void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths) = 0;
            
            virtual Map* doNewMap(MapFormat::Type format) const = 0;
            virtual Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const = 0;
            virtual Model::EntityList doParseEntities(const BBox3& worldBounds, const String& str) const = 0;
            virtual Model::BrushList doParseBrushes(const BBox3& worldBounds, const String& str) const = 0;
            virtual Model::BrushFaceList doParseFaces(const BBox3& worldBounds, const String& str) const = 0;
            
            virtual void doWriteMap(Map& map, const IO::Path& path) const = 0;
            virtual void doWriteObjectsToStream(MapFormat::Type format, const Model::ObjectList& objects, std::ostream& stream) const = 0;
            virtual void doWriteFacesToStream(MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) const = 0;
            
            virtual IO::Path::List doFindBuiltinTextureCollections() const = 0;
            virtual IO::Path::List doExtractTexturePaths(const Map* map) const = 0;
            virtual Assets::TextureCollection* doLoadTextureCollection(const IO::Path& path) const = 0;
            
            virtual Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const = 0;
            virtual IO::Path::List doAllEntityDefinitionFiles() const = 0;
            virtual IO::Path doExtractEntityDefinitionFile(const Map* map) const = 0;
            virtual Assets::EntityModel* doLoadModel(const IO::Path& path) const = 0;
            
            virtual StringList doAvailableMods() const = 0;
            virtual StringList doExtractEnabledMods(const Map* map) const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Game__) */
