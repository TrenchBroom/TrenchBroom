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

#ifndef __TrenchBroom__GameImpl__
#define __TrenchBroom__GameImpl__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/GameFileSystem.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/ModelTypes.h"


namespace TrenchBroom {
    namespace Model {
        class GameImpl : public Game {
        private:
            typedef std::tr1::shared_ptr<IO::MapWriter> MapWriterPtr;
            
            GameConfig m_config;
            IO::Path m_gamePath;
            IO::Path::List m_additionalSearchPaths;
            
            IO::GameFileSystem m_fs;
            Assets::Palette* m_palette;
        public:
            GameImpl(const GameConfig& config, const IO::Path& gamePath);
            ~GameImpl();
        private:
            const String& doGameName() const;
            IO::Path doGamePath() const;
            void doSetGamePath(const IO::Path& gamePath);
            void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths);

            Map* doNewMap(MapFormat::Type format) const;
            Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const;
            EntityList doParseEntities(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            BrushList doParseBrushes(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            BrushFaceList doParseFaces(const BBox3& worldBounds, MapFormat::Type format, const String& str) const;
            
            void doWriteMap(Map& map, const IO::Path& path) const;
            void doWriteObjectsToStream(MapFormat::Type format, const ObjectList& objects, std::ostream& stream) const;
            void doWriteFacesToStream(MapFormat::Type format, const BrushFaceList& faces, std::ostream& stream) const;
            
            bool doIsTextureCollection(const IO::Path& path) const;
            IO::Path::List doFindBuiltinTextureCollections() const;
            StringList doExtractExternalTextureCollections(const Map* map) const;
            void doUpdateExternalTextureCollections(Map* map, const StringList& collections) const;
            Assets::TextureCollection* doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const;
            
            bool doIsEntityDefinitionFile(const IO::Path& path) const;
            Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const;
            EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const;
            EntityDefinitionFileSpec doExtractEntityDefinitionFile(const Map* map) const;
            EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
            IO::Path doFindEntityDefinitionFile(const EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const;
            Assets::EntityModel* doLoadModel(const IO::Path& path) const;

            MapWriterPtr mapWriter(MapFormat::Type format) const;
            
            Assets::TextureCollection* loadWadTextureCollection(const Assets::TextureCollectionSpec& spec) const;
            Assets::TextureCollection* loadWalTextureCollection(const Assets::TextureCollectionSpec& spec) const;
            
            Assets::EntityModel* loadBspModel(const String& name, const IO::MappedFile::Ptr file) const;
            Assets::EntityModel* loadMdlModel(const String& name, const IO::MappedFile::Ptr file) const;
            Assets::EntityModel* loadMd2Model(const String& name, const IO::MappedFile::Ptr file) const;
            
            StringList doAvailableMods() const;
            StringList doExtractEnabledMods(const Map* map) const;
            
            const GameConfig::FlagsConfig& doSurfaceFlags() const;
            const GameConfig::FlagsConfig& doContentFlags() const;
        };
    }
}

#endif /* defined(__TrenchBroom__GameImpl__) */
