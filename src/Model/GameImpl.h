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
            void doSetGamePath(const IO::Path& gamePath);
            void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths);

            Map* doNewMap(MapFormat::Type format) const;
            Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const;
            Model::EntityList doParseEntities(const BBox3& worldBounds, const String& str) const;
            Model::BrushList doParseBrushes(const BBox3& worldBounds, const String& str) const;
            Model::BrushFaceList doParseFaces(const BBox3& worldBounds, const String& str) const;
            
            void doWriteMap(Map& map, const IO::Path& path) const;
            void doWriteObjectsToStream(MapFormat::Type format, const Model::ObjectList& objects, std::ostream& stream) const;
            void doWriteFacesToStream(MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) const;
            
            IO::Path::List doFindBuiltinTextureCollections() const;
            IO::Path::List doExtractTexturePaths(const Map* map) const;
            Assets::TextureCollection* doLoadTextureCollection(const IO::Path& path) const;
            
            Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const;
            IO::Path doDefaultEntityDefinitionFile() const;
            IO::Path doExtractEntityDefinitionFile(const Map* map) const;
            Assets::EntityModel* doLoadModel(const IO::Path& path) const;

            MapWriterPtr mapWriter(MapFormat::Type format) const;
            
            Assets::TextureCollection* loadWadTextureCollection(const IO::Path& path) const;
            Assets::TextureCollection* loadWalTextureCollection(const IO::Path& path) const;
            
            Assets::EntityModel* loadBspModel(const String& name, const IO::MappedFile::Ptr file) const;
            Assets::EntityModel* loadMdlModel(const String& name, const IO::MappedFile::Ptr file) const;
            Assets::EntityModel* loadMd2Model(const String& name, const IO::MappedFile::Ptr file) const;
            
            StringList doExtractMods(const Map* map) const;
        };
    }
}

#endif /* defined(__TrenchBroom__GameImpl__) */
