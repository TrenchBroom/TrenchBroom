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
#include "IO/FileSystem.h"
#include "IO/QuakeMapParser.h"
#include "IO/WadTextureLoader.h"
#include "Model/GameUtils.h"

namespace TrenchBroom {
    namespace Model {
        GamePtr Hexen2Game::newGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) {
            return GamePtr(new Hexen2Game(gamePath, defaultEntityColor, logger));
        }
        
        Hexen2Game::Hexen2Game(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger) :
        Game(logger),
        m_fs(gamePath, IO::Path("data1")),
        m_defaultEntityColor(defaultEntityColor),
        m_palette(palettePath()) {}
        
        const BBox3 Hexen2Game::WorldBounds = BBox3(-8192.0, 8192.0);
        
        IO::Path Hexen2Game::palettePath() {
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("hexen2/palette.lmp");
        }
        
        Map* Hexen2Game::doNewMap() const {
            return new Map(MFHexen2);
        }

        Map* Hexen2Game::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            IO::FileSystem fs;
            IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
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
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("hexen2/Hexen2.fgd");
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