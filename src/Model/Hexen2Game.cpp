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

#include "Hexen2Game.h"
#include "IO/DiskFileSystem.h"
#include "IO/Hexen2MapWriter.h"
#include "IO/QuakeMapParser.h"
#include "IO/SystemPaths.h"
#include "IO/WadTextureLoader.h"
#include "Model/GameUtils.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        GamePtr Hexen2Game::newGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) {
            return GamePtr(new Hexen2Game(gamePath, defaultEntityColor, logger));
        }
        
        Hexen2Game::Hexen2Game(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) :
        Game(logger),
        m_fs("pak", gamePath + IO::Path("data1")),
        m_defaultEntityColor(defaultEntityColor),
        m_palette(palettePath()) {}
        
        const BBox3 Hexen2Game::WorldBounds = BBox3(-8192.0, 8192.0);
        
        IO::Path Hexen2Game::palettePath() {
            return IO::SystemPaths::resourceDirectory() + IO::Path("hexen2/palette.lmp");
        }
        
        Map* Hexen2Game::doNewMap() const {
            return new Map(MFHexen2);
        }

        Map* Hexen2Game::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
        }
        
        Model::EntityList Hexen2Game::doParseEntities(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseEntities(worldBounds);
        }
        
        Model::BrushList Hexen2Game::doParseBrushes(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseBrushes(worldBounds);
        }
        
        Model::BrushFaceList Hexen2Game::doParseFaces(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseFaces(worldBounds);
        }

        void Hexen2Game::doWriteMap(Map& map, const IO::Path& path) const {
            IO::Hexen2MapWriter writer;
            writer.writeToFileAtPath(map, path, true);
        }

        void Hexen2Game::doWriteObjectsToStream(const Model::ObjectList& objects, std::ostream& stream) const {
            IO::Hexen2MapWriter writer;
            writer.writeObjectsToStream(objects, stream);
        }
        
        void Hexen2Game::doWriteFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream) const {
            IO::Hexen2MapWriter writer;
            writer.writeFacesToStream(faces, stream);
        }
        
        IO::Path::List Hexen2Game::doExtractTexturePaths(const Map* map) const {
            using TrenchBroom::Model::extractTexturePaths;
            return extractTexturePaths(map, Model::PropertyKeys::Wad);
        }
        
        Assets::FaceTextureCollection* Hexen2Game::doLoadTextureCollection(const IO::Path& path) const {
            IO::WadTextureLoader loader(m_palette);
            return loader.loadTextureCollection(path);
        }
        
        void Hexen2Game::doUploadTextureCollection(Assets::FaceTextureCollection* collection) const {
            IO::WadTextureLoader loader(m_palette);
            return loader.uploadTextureCollection(collection);
        }
        
        Assets::EntityDefinitionList Hexen2Game::doLoadEntityDefinitions(const IO::Path& path) const {
            using TrenchBroom::Model::loadEntityDefinitions;
            return loadEntityDefinitions(path, m_defaultEntityColor);
        }
        
        IO::Path Hexen2Game::doDefaultEntityDefinitionFile() const {
            return IO::SystemPaths::resourceDirectory() + IO::Path("hexen2/Hexen2.fgd");
        }
        
        IO::Path Hexen2Game::doExtractEntityDefinitionFile(const Map* map) const {
            using TrenchBroom::Model::extractEntityDefinitionFile;
            return extractEntityDefinitionFile(map, defaultEntityDefinitionFile());
        }
        
        Assets::EntityModel* Hexen2Game::doLoadModel(const IO::Path& path) const {
            using TrenchBroom::Model::loadModel;
            return loadModel(m_fs, m_palette, path);
        }
    }
}