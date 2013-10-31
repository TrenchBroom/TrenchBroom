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

#include "QuakeGame.h"

#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/QuakeMapParser.h"
#include "IO/QuakeMapWriter.h"
#include "IO/SystemPaths.h"
#include "IO/WadTextureLoader.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/GameUtils.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        GamePtr QuakeGame::newGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) {
            return GamePtr(new QuakeGame(gamePath, defaultEntityColor, logger));
        }

        QuakeGame::QuakeGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) :
        Game(logger),
        m_fs("pak", gamePath + IO::Path("id1")),
        m_defaultEntityColor(defaultEntityColor),
        m_palette(palettePath()) {}
        
        const BBox3 QuakeGame::WorldBounds = BBox3(Vec3(-16384.0, -16384.0, -16384.0),
                                                   Vec3(+16384.0, +16384.0, +16384.0));
        
        IO::Path QuakeGame::palettePath() {
            return IO::SystemPaths::resourceDirectory() + IO::Path("quake/palette.lmp");
        }

        Map* QuakeGame::doNewMap() const {
            return new Map(MFQuake);
        }

        Map* QuakeGame::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
        }

        Model::EntityList QuakeGame::doParseEntities(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseEntities(worldBounds);
        }
        
        Model::BrushList QuakeGame::doParseBrushes(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseBrushes(worldBounds);
        }
        
        Model::BrushFaceList QuakeGame::doParseFaces(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseFaces(worldBounds);
        }
        void QuakeGame::doWriteMap(Map& map, const IO::Path& path) const {
            IO::QuakeMapWriter writer;
            writer.writeToFileAtPath(map, path, true);
        }

        void QuakeGame::doWriteObjectsToStream(const Model::ObjectList& objects, std::ostream& stream) const {
            IO::QuakeMapWriter writer;
            writer.writeObjectsToStream(objects, stream);
        }
        
        void QuakeGame::doWriteFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream) const {
            IO::QuakeMapWriter writer;
            writer.writeFacesToStream(faces, stream);
        }

        IO::Path::List QuakeGame::doFindBuiltinTextureCollections() const {
            return IO::Path::List();
        }

        IO::Path::List QuakeGame::doExtractTexturePaths(const Map* map) const {
            using TrenchBroom::Model::extractTexturePaths;
            return extractTexturePaths(map, Model::PropertyKeys::Wad);
        }

        Assets::TextureCollection* QuakeGame::doLoadTextureCollection(const IO::Path& path) const {
            IO::WadTextureLoader loader(m_palette);
            return loader.loadTextureCollection(path);
        }

        void QuakeGame::doUploadTextureCollection(Assets::TextureCollection* collection) const {
            IO::WadTextureLoader loader(m_palette);
            loader.uploadTextureCollection(collection);
        }

        Assets::EntityDefinitionList QuakeGame::doLoadEntityDefinitions(const IO::Path& path) const {
            using TrenchBroom::Model::loadEntityDefinitions;
            return loadEntityDefinitions(path, m_defaultEntityColor);
        }

        IO::Path QuakeGame::doDefaultEntityDefinitionFile() const {
            return IO::SystemPaths::resourceDirectory() + IO::Path("quake/Quake.fgd");
        }

        IO::Path QuakeGame::doExtractEntityDefinitionFile(const Map* map) const {
            using TrenchBroom::Model::extractEntityDefinitionFile;
            return extractEntityDefinitionFile(map, defaultEntityDefinitionFile());
        }

        Assets::EntityModel* QuakeGame::doLoadModel(const IO::Path& path) const {
            using TrenchBroom::Model::loadModel;
            return loadModel(m_fs, m_palette, path);
        }
    }
}
