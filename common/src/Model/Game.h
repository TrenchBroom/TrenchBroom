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

#ifndef __TrenchBroom__Game__
#define __TrenchBroom__Game__

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
            World* newMap(MapFormat::Type format) const;
            World* loadMap(const BBox3& worldBounds, const IO::Path& path) const;
            void writeMap(World* world, const IO::Path& path) const;
        public: // parsing and serializing objects
            /*
            EntityList parseEntities(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            BrushList parseBrushes(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            BrushFaceList parseFaces(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            void writeObjectsToStream(MapFormat::Type format, const ObjectList& objects, std::ostream& stream) const;
            void writeFacesToStream(MapFormat::Type format, const BrushFaceList& faces, std::ostream& stream) const;
             */
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
            
            virtual World* doNewMap(MapFormat::Type format) const = 0;
            virtual World* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const = 0;
            virtual void doWriteMap(World* world, const IO::Path& path) const = 0;
            
            /*
            virtual EntityList doParseEntities(const BBox3& worldBounds, MapFormat::Type format, const String& str) const = 0;
            virtual BrushList doParseBrushes(const BBox3& worldBounds, MapFormat::Type format, const String& str) const = 0;
            virtual BrushFaceList doParseFaces(const BBox3& worldBounds, MapFormat::Type format, const String& str) const = 0;
            virtual void doWriteObjectsToStream(MapFormat::Type format, const ObjectList& objects, std::ostream& stream) const = 0;
            virtual void doWriteFacesToStream(MapFormat::Type format, const BrushFaceList& faces, std::ostream& stream) const = 0;
             */
            
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

#endif /* defined(__TrenchBroom__Game__) */
