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

#include "GameConfigParser.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        GameConfigParser::GameConfigParser(const char* begin, const char* end) :
        m_parser(begin, end) {}
        
        GameConfigParser::GameConfigParser(const String& str) :
        m_parser(str) {}
        
        Model::GameConfig GameConfigParser::parse() {
            using Model::GameConfig;

            const ConfigEntry::Ptr root = m_parser.parse();
            if (root == NULL)
                throw ParserException("Empty game config");
            
            expectEntry(ConfigEntry::TTable, *root);
            const ConfigTable& rootTable = *root;
            
            expectTableEntry("name", ConfigEntry::TValue, rootTable);
            const String name = rootTable["name"];
            
            expectTableEntry("fileformats", ConfigEntry::TList, rootTable);
            const StringSet fileFormats = parseList(rootTable["fileformats"]);
            
            expectTableEntry("filesystem", ConfigEntry::TTable, rootTable);
            const GameConfig::FileSystemConfig fileSystemConfig = parseFileSystemConfig(rootTable["filesystem"]);
            
            expectTableEntry("textures", ConfigEntry::TTable, rootTable);
            const GameConfig::TextureConfig textureConfig = parseTextureConfig(rootTable["textures"]);
            
            expectTableEntry("entities", ConfigEntry::TTable, rootTable);
            const GameConfig::EntityConfig entityConfig = parseEntityConfig(rootTable["entities"]);
            
            return GameConfig(name, fileFormats, fileSystemConfig, textureConfig, entityConfig);
        }

        Model::GameConfig::FileSystemConfig GameConfigParser::parseFileSystemConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            expectTableEntry("searchpath", ConfigEntry::TValue, table);
            const String searchPath = table["searchpath"];
            
            expectTableEntry("packageformat", ConfigEntry::TValue, table);
            const String packageFormat = table["packageformat"];
            
            return GameConfig::FileSystemConfig(Path(searchPath), packageFormat);
        }

        Model::GameConfig::TextureConfig GameConfigParser::parseTextureConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            expectTableEntry("type", ConfigEntry::TValue, table);
            const String type = table["type"];

            String property("");
            if (table.contains("property")) {
                expectTableEntry("property", ConfigEntry::TValue, table);
                property = table["property"];
            }
            
            IO::Path palette("");
            if (table.contains("palette")) {
                expectTableEntry("palette", ConfigEntry::TValue, table);
                palette = IO::Path(table["palette"]);
            }
            
            IO::Path builtinTexturesSearchPath("");
            if (table.contains("builtin")) {
                expectTableEntry("builtin", ConfigEntry::TValue, table);
                builtinTexturesSearchPath = IO::Path(table["builtin"]);
            }
            
            return GameConfig::TextureConfig(type, property, IO::Path(palette), builtinTexturesSearchPath);
        }

        Model::GameConfig::EntityConfig GameConfigParser::parseEntityConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            expectTableEntry("definitions", ConfigEntry::TValue, table);
            const String defFilePath = table["definitions"];
            
            expectTableEntry("modelformats", ConfigEntry::TList, table);
            const StringSet modelFormats = parseList(table["modelformats"]);
            
            expectTableEntry("defaultcolor", ConfigEntry::TValue, table);
            const Color defaultColor(table["defaultcolor"]);
            
            return GameConfig::EntityConfig(Path(defFilePath), modelFormats, defaultColor);
        }

        StringSet GameConfigParser::parseList(const ConfigList& list) const {
            StringSet result;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::TValue, entry);
                result.insert(static_cast<const String&>(entry));
            }
            return result;
        }

        void GameConfigParser::expectEntry(const ConfigEntry::Type typeMask, const ConfigEntry& entry) const {
            if ((typeMask & entry.type()) == 0)
                throw ParserException("Expected " + typeNames(typeMask) + ", but got " + typeNames(entry.type()));
        }

        void GameConfigParser::expectTableEntry(const String& key, const ConfigEntry::Type typeMask, const ConfigTable& parent) const {
            if (!parent.contains(key))
                throw ParserException("Expected table entry '" + key + "' with type " + typeNames(typeMask));
            if ((parent[key].type() & typeMask) == 0)
                throw ParserException("Expected table entry '" + key + "' with type " + typeNames(typeMask) + ", but got table entry with type '" + typeNames(parent[key].type()) + "'");
        }
        
        String GameConfigParser::typeNames(const ConfigEntry::Type typeMask) const {
            StringList result;
            if ((typeMask & ConfigEntry::TValue) != 0)
                result.push_back("value");
            if ((typeMask & ConfigEntry::TList) != 0)
                result.push_back("list");
            if ((typeMask & ConfigEntry::TTable) != 0)
                result.push_back("table");
            
            if (result.empty())
                return "none";
            if (result.size() == 1)
                return result.front();
            return StringUtils::join(result, ", ", ", or ", " or ");
        }
    }
}
