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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "QuakeGame.h"

#include "StringUtils.h"
#include "IO/DefParser.h"
#include "IO/FileSystem.h"
#include "IO/FgdParser.h"
#include "IO/Path.h"
#include "IO/QuakeMapParser.h"
#include "IO/WadTextureLoader.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Map.h"
#include "View/Logger.h"

namespace TrenchBroom {
    namespace Model {
        GamePtr QuakeGame::newGame(const Color& defaultEntityColor, View::Logger* logger) {
            return GamePtr(new QuakeGame(defaultEntityColor, logger));
        }

        QuakeGame::QuakeGame(const Color& defaultEntityColor, View::Logger* logger) :
        m_defaultEntityColor(defaultEntityColor),
        m_logger(logger),
        m_palette(palettePath()) {}
        
        const BBox3 QuakeGame::WorldBounds = BBox3(Vec3(-16384.0, -16384.0, -16384.0),
                                                   Vec3(+16384.0, +16384.0, +16384.0));
        
        IO::Path QuakeGame::palettePath() {
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("quake/palette.lmp");
        }

        Map* QuakeGame::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            IO::FileSystem fs;
            IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
            IO::QuakeMapParser parser(file->begin(), file->end(), m_logger);
            return parser.parseMap(worldBounds);
        }

        IO::Path::List QuakeGame::doExtractTexturePaths(const Map* map) const {
            IO::Path::List paths;
            
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return paths;
            
            const Model::PropertyValue& wadValue = worldspawn->property(Model::PropertyKeys::Wad);
            if (wadValue.empty())
                return paths;
            
            const StringList pathStrs = StringUtils::split(wadValue, ';');
            StringList::const_iterator it, end;
            for (it = pathStrs.begin(), end = pathStrs.end(); it != end; ++it) {
                const String pathStr = StringUtils::trim(*it);
                if (!pathStr.empty()) {
                    const IO::Path path(pathStr);
                    paths.push_back(path);
                }
            }
            
            return paths;
        }

        Assets::FaceTextureCollection* QuakeGame::doLoadTextureCollection(const IO::Path& path) const {
            IO::WadTextureLoader loader(m_palette);
            return loader.loadTextureCollection(path);
        }

        void QuakeGame::doUploadTextureCollection(Assets::FaceTextureCollection* collection) const {
            IO::WadTextureLoader loader(m_palette);
            loader.uploadTextureCollection(collection);
        }

        Assets::EntityDefinitionList QuakeGame::doLoadEntityDefinitions(const IO::Path& path) const {
            const String extension = path.extension();
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                IO::FileSystem fs;
                IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
                IO::FgdParser parser(file->begin(), file->end(), m_defaultEntityColor);
                return parser.parseDefinitions();
            }
            if (StringUtils::caseInsensitiveEqual("def", extension)) {
                IO::FileSystem fs;
                IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
                IO::DefParser parser(file->begin(), file->end(), m_defaultEntityColor);
                return parser.parseDefinitions();
            }
            throw GameException("Unknown entity definition format: " + path.asString());
        }

        IO::Path QuakeGame::doDefaultEntityDefinitionFile() const {
            IO::FileSystem fs;
            return fs.resourceDirectory() + IO::Path("quake/Quake.fgd");
        }

        IO::Path QuakeGame::doExtractEntityDefinitionFile(const Map* map) const {
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return defaultEntityDefinitionFile();
            
            const Model::PropertyValue& defValue = worldspawn->property(Model::PropertyKeys::EntityDefinitions);
            if (defValue.empty())
                return defaultEntityDefinitionFile();
            
            if (StringUtils::isPrefix(defValue, "external:"))
                return IO::Path(defValue.substr(9));
            if (StringUtils::isPrefix(defValue, "builtin:")) {
                IO::FileSystem fs;
                return fs.resourceDirectory() + IO::Path(defValue.substr(8));
            }
            
            const IO::Path defPath(defValue);
            if (defPath.isAbsolute())
                return defPath;

            IO::FileSystem fs;
            return fs.resourceDirectory() + defPath;
        }

        Assets::EntityModel* QuakeGame::doLoadModel(const IO::Path& path) const {
            // search file system for model file under Quake path
            // search pak files under Quake path
        }
    }
}
