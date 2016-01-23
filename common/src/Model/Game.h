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

#ifndef TrenchBroom_Game
#define TrenchBroom_Game

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/EntityDefinitionLoader.h"
#include "IO/EntityModelLoader.h"
#include "IO/GameFileSystem.h"
#include "IO/TextureLoader.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class MapWriter;
    }
    
    namespace Model {
        class BrushContentTypeBuilder;
        
        class Game : public IO::EntityDefinitionLoader, public IO::EntityModelLoader, public IO::TextureLoader {
        private:
            mutable BrushContentTypeBuilder* m_brushContentTypeBuilder;
        protected:
            Game();
        public:
            virtual ~Game();
        public:
            const String& gameName() const;
            bool isGamePathPreference(const IO::Path& prefPath) const;
            
            IO::Path gamePath() const;
            void setGamePath(const IO::Path& gamePath);
            void setAdditionalSearchPaths(const IO::Path::List& searchPaths);
        public: // loading and writing map files
            World* newMap(MapFormat::Type format, const BBox3& worldBounds) const;
            World* loadMap(MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const;
            void writeMap(World* world, const IO::Path& path) const;
        public: // parsing and serializing objects
            NodeList parseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;
            BrushFaceList parseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;

            void writeNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const;
            void writeBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const;
        public: // texture collection handling
            bool isTextureCollection(const IO::Path& path) const;
            IO::Path::List findBuiltinTextureCollections() const;
            StringList extractExternalTextureCollections(const World* world) const;
            void updateExternalTextureCollections(World* world, const StringList& collections) const;
        public: // entity definition handling
            bool isEntityDefinitionFile(const IO::Path& path) const;
            Assets::EntityDefinitionFileSpec::List allEntityDefinitionFiles() const;
            Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(const World* world) const;
            IO::Path findEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const;
        public: // brush content type
            const BrushContentTypeBuilder* brushContentTypeBuilder() const;
            const BrushContentType::List& brushContentTypes() const;
        public: // mods
            StringList availableMods() const;
            StringList extractEnabledMods(const World* world) const;
        public: // flag configs for faces
            const GameConfig::FlagsConfig& surfaceFlags() const;
            const GameConfig::FlagsConfig& contentFlags() const;
        private: // subclassing interface
            virtual const String& doGameName() const = 0;
            virtual IO::Path doGamePath() const = 0;
            virtual void doSetGamePath(const IO::Path& gamePath) = 0;
            virtual void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths) = 0;
            
            virtual World* doNewMap(MapFormat::Type format, const BBox3& worldBounds) const = 0;
            virtual World* doLoadMap(MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const = 0;
            virtual void doWriteMap(World* world, const IO::Path& path) const = 0;
            
            virtual NodeList doParseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const = 0;
            virtual BrushFaceList doParseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const = 0;
            virtual void doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const = 0;
            virtual void doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const = 0;
            
            virtual bool doIsTextureCollection(const IO::Path& path) const = 0;
            virtual IO::Path::List doFindBuiltinTextureCollections() const = 0;
            virtual StringList doExtractExternalTextureCollections(const World* world) const = 0;
            virtual void doUpdateExternalTextureCollections(World* world, const StringList& collections) const = 0;
            
            virtual bool doIsEntityDefinitionFile(const IO::Path& path) const = 0;
            virtual Assets::EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const = 0;
            virtual Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const World* world) const = 0;
            virtual IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const = 0;
            
            virtual const BrushContentType::List& doBrushContentTypes() const = 0;
            
            virtual StringList doAvailableMods() const = 0;
            virtual StringList doExtractEnabledMods(const World* world) const = 0;

            virtual const GameConfig::FlagsConfig& doSurfaceFlags() const = 0;
            virtual const GameConfig::FlagsConfig& doContentFlags() const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Game) */
