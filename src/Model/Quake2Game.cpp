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
#include "IO/FileSystem.h"
#include "IO/QuakeMapParser.h"
#include "IO/Quake2MapWriter.h"
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
        m_fs(gamePath, IO::Path("baseq2")),
        m_defaultEntityColor(defaultEntityColor),
        m_palette(palettePath()) {}
        
        const BBox3 Quake2Game::WorldBounds = BBox3(Vec3(-16384.0, -16384.0, -16384.0),
                                                   Vec3(+16384.0, +16384.0, +16384.0));
        
        IO::Path Quake2Game::palettePath() {
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("quake2/colormap.pcx");
        }
        
        Map* Quake2Game::doNewMap() const {
            return new Map(MFQuake2);
        }

        Map* Quake2Game::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            IO::FileSystem fs;
            IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
        }
        
        void Quake2Game::doWriteMap(Map& map, const IO::Path& path) const {
            IO::Quake2MapWriter writer;
            writer.writeToFileAtPath(map, path, true);
        }

        IO::Path::List Quake2Game::doExtractTexturePaths(const Map* map) const {
            using TrenchBroom::Model::extractTexturePaths;
            return extractTexturePaths(map, Model::PropertyKeys::Wal);
        }
        
        Assets::FaceTextureCollection* Quake2Game::doLoadTextureCollection(const IO::Path& path) const {
            IO::WalTextureLoader loader(m_palette);
            return loader.loadTextureCollection(path);
        }
        
        void Quake2Game::doUploadTextureCollection(Assets::FaceTextureCollection* collection) const {
            IO::WalTextureLoader loader(m_palette);
            loader.uploadTextureCollection(collection);
        }
        
        Assets::EntityDefinitionList Quake2Game::doLoadEntityDefinitions(const IO::Path& path) const {
            using TrenchBroom::Model::loadEntityDefinitions;
            return loadEntityDefinitions(path, m_defaultEntityColor);
        }
        
        IO::Path Quake2Game::doDefaultEntityDefinitionFile() const {
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("quake2/Quake2.fgd");
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
