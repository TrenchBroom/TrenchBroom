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

#include "GameUtils.h"

#include "Exceptions.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/FgdParser.h"
#include "IO/GameFileSystem.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/SystemPaths.h"
#include "Model/Entity.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        IO::Path::List extractTexturePaths(const Map* map, const Model::PropertyKey& key) {
            IO::Path::List paths;
            
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return paths;
            
            const Model::PropertyValue& pathsValue = worldspawn->property(key);
            if (pathsValue.empty())
                return paths;
            
            const StringList pathStrs = StringUtils::split(pathsValue, ';');
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
        
        Assets::EntityDefinitionList loadEntityDefinitions(const IO::Path& path, const Color& defaultEntityColor) {
            const String extension = path.extension();
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::FgdParser parser(file->begin(), file->end(), defaultEntityColor);
                return parser.parseDefinitions();
            }
            if (StringUtils::caseInsensitiveEqual("def", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::DefParser parser(file->begin(), file->end(), defaultEntityColor);
                return parser.parseDefinitions();
            }
            throw GameException("Unknown entity definition format: " + path.asString());
        }
        
        IO::Path extractEntityDefinitionFile(const Map* map, const IO::Path& defaultFile) {
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return defaultFile;
            
            const Model::PropertyValue& defValue = worldspawn->property(Model::PropertyKeys::EntityDefinitions);
            if (defValue.empty())
                return defaultFile;
            
            if (StringUtils::isPrefix(defValue, "external:"))
                return IO::Path(defValue.substr(9));
            if (StringUtils::isPrefix(defValue, "builtin:"))
                return IO::SystemPaths::resourceDirectory() + IO::Path(defValue.substr(8));
            
            const IO::Path defPath(defValue);
            if (defPath.isAbsolute())
                return defPath;
            return IO::SystemPaths::resourceDirectory() + defPath;
        }
        
        Assets::EntityModel* loadModel(const IO::GameFileSystem& gameFS, const Assets::Palette& palette, const IO::Path& path) {
            IO::MappedFile::Ptr file = gameFS.openFile(path);
            if (file == NULL)
                return NULL;
            
            if (StringUtils::caseInsensitiveEqual(path.extension(), "mdl")) {
                IO::MdlParser parser(path.lastComponent().asString(), file->begin(), file->end(), palette);
                return parser.parseModel();
            } else if (StringUtils::caseInsensitiveEqual(path.extension(), "md2")) {
                IO::Md2Parser parser(path.lastComponent().asString(), file->begin(), file->end(), palette, gameFS);
                return parser.parseModel();
            } else if (StringUtils::caseInsensitiveEqual(path.extension(), "bsp")) {
                IO::Bsp29Parser parser(path.lastComponent().asString(), file->begin(), file->end(), palette);
                return parser.parseModel();
            } else {
                throw GameException("Unknown model type " + path.asString());
            }
            
            return NULL;
        }
    }
}
