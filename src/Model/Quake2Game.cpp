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

#include "Quake2Game.h"
#include "IO/DiskFileSystem.h"
#include "IO/QuakeMapParser.h"
#include "IO/Quake2MapWriter.h"
#include "IO/SystemPaths.h"
#include "IO/WalTextureLoader.h"
#include "Model/GameUtils.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        GamePtr Quake2Game::newGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) {
            return GamePtr(new Quake2Game(gamePath, defaultEntityColor, logger));
        }
        
        Quake2Game::Quake2Game(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) :
        Game(logger),
        m_fs("pak", gamePath + IO::Path("baseq2")),
        m_defaultEntityColor(defaultEntityColor),
        m_palette(palettePath()) {}
        
        const BBox3 Quake2Game::WorldBounds = BBox3(Vec3(-16384.0, -16384.0, -16384.0),
                                                   Vec3(+16384.0, +16384.0, +16384.0));
        
        IO::Path Quake2Game::palettePath() {
            return IO::SystemPaths::resourceDirectory() + IO::Path("quake2/colormap.pcx");
        }
        
        Map* Quake2Game::doNewMap() const {
            return new Map(MFQuake2);
        }

        Map* Quake2Game::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
        }
        
        Model::EntityList Quake2Game::doParseEntities(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseEntities(worldBounds);
        }
        
        Model::BrushList Quake2Game::doParseBrushes(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseBrushes(worldBounds);
        }
        
        Model::BrushFaceList Quake2Game::doParseFaces(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseFaces(worldBounds);
        }

        void Quake2Game::doWriteMap(Map& map, const IO::Path& path) const {
            IO::Quake2MapWriter writer;
            writer.writeToFileAtPath(map, path, true);
        }

        void Quake2Game::doWriteObjectsToStream(const Model::ObjectList& objects, std::ostream& stream) const {
            IO::Quake2MapWriter writer;
            writer.writeObjectsToStream(objects, stream);
        }
        
        void Quake2Game::doWriteFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream) const {
            IO::Quake2MapWriter writer;
            writer.writeFacesToStream(faces, stream);
        }

        IO::Path::List Quake2Game::doFindBuiltinTextureCollections() const {
            return m_fs.findItems(IO::Path("textures"), IO::FileSystem::TypeMatcher(false, true));
        }

        IO::Path::List Quake2Game::doExtractTexturePaths(const Map* map) const {
            using TrenchBroom::Model::extractTexturePaths;
            return extractTexturePaths(map, Model::PropertyKeys::Wal);
        }
        
        Assets::TextureCollection* Quake2Game::doLoadTextureCollection(const IO::Path& path) const {
            if (path.isAbsolute()) {
                IO::DiskFileSystem diskFS(path.deleteLastComponent());
                IO::WalTextureLoader loader(diskFS, m_palette);
                return loader.loadTextureCollection(path.lastComponent());
            } else {
                IO::WalTextureLoader loader(m_fs, m_palette);
                return loader.loadTextureCollection(path);
            }
        }
        
        Assets::EntityDefinitionList Quake2Game::doLoadEntityDefinitions(const IO::Path& path) const {
            using TrenchBroom::Model::loadEntityDefinitions;
            return loadEntityDefinitions(path, m_defaultEntityColor);
        }
        
        IO::Path Quake2Game::doDefaultEntityDefinitionFile() const {
            return IO::SystemPaths::resourceDirectory() + IO::Path("quake2/Quake2.fgd");
        }
        
        IO::Path Quake2Game::doExtractEntityDefinitionFile(const Map* map) const {
            using TrenchBroom::Model::extractEntityDefinitionFile;
            return extractEntityDefinitionFile(map, defaultEntityDefinitionFile());
        }

        Assets::EntityModel* Quake2Game::doLoadModel(const IO::Path& path) const {
            using TrenchBroom::Model::loadModel;
            return loadModel(m_fs, m_palette, path);
        }
    }
}
