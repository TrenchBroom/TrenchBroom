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

#include "GameConfigParser.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        GameConfigParser::GameConfigParser(const char* begin, const char* end, const Path& path) :
        m_parser(begin, end),
        m_path(path) {}
        
        GameConfigParser::GameConfigParser(const String& str, const Path& path) :
        m_parser(str),
        m_path(path) {}
        
        Model::GameConfig GameConfigParser::parse() {
            using Model::GameConfig;

            const ConfigEntry::Ptr root = m_parser.parse();
            if (root == NULL)
                throw ParserException("Empty game config");
            
            expectEntry(ConfigEntry::Type_Table, *root);
            const ConfigTable& rootTable = *root;
            
            expectTableEntry("name", ConfigEntry::Type_Value, rootTable);
            const String name = rootTable["name"];
            
            IO::Path icon("");
            if (rootTable.contains("icon")) {
                expectTableEntry("icon", ConfigEntry::Type_Value, rootTable);
                icon = IO::Path(rootTable["icon"]);
            }
            
            expectTableEntry("fileformats", ConfigEntry::Type_List, rootTable);
            const StringSet fileFormats = parseSet(rootTable["fileformats"]);
            
            expectTableEntry("filesystem", ConfigEntry::Type_Table, rootTable);
            const GameConfig::FileSystemConfig fileSystemConfig = parseFileSystemConfig(rootTable["filesystem"]);
            
            expectTableEntry("textures", ConfigEntry::Type_Table, rootTable);
            const GameConfig::TextureConfig textureConfig = parseTextureConfig(rootTable["textures"]);
            
            expectTableEntry("entities", ConfigEntry::Type_Table, rootTable);
            const GameConfig::EntityConfig entityConfig = parseEntityConfig(rootTable["entities"]);
            
            GameConfig::FaceAttribsConfig faceAttribsConfig;
            if (rootTable.contains("faceattribs")) {
                expectTableEntry("faceattribs", ConfigEntry::Type_Table, rootTable);
                faceAttribsConfig = parseFaceAttribsConfig(rootTable["faceattribs"]);
            }
            
            return GameConfig(name, m_path, icon, fileFormats, fileSystemConfig, textureConfig, entityConfig, faceAttribsConfig);
        }

        Model::GameConfig::FileSystemConfig GameConfigParser::parseFileSystemConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            expectTableEntry("searchpath", ConfigEntry::Type_Value, table);
            const String searchPath = table["searchpath"];
            
            expectTableEntry("packageformat", ConfigEntry::Type_Value, table);
            const String packageFormat = table["packageformat"];
            
            return GameConfig::FileSystemConfig(Path(searchPath), packageFormat);
        }

        Model::GameConfig::TextureConfig GameConfigParser::parseTextureConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            expectTableEntry("type", ConfigEntry::Type_Value, table);
            const String type = table["type"];

            String property("");
            if (table.contains("property")) {
                expectTableEntry("property", ConfigEntry::Type_Value, table);
                property = table["property"];
            }
            
            IO::Path palette("");
            if (table.contains("palette")) {
                expectTableEntry("palette", ConfigEntry::Type_Value, table);
                palette = IO::Path(table["palette"]);
            }
            
            IO::Path builtinTexturesSearchPath("");
            if (table.contains("builtin")) {
                expectTableEntry("builtin", ConfigEntry::Type_Value, table);
                builtinTexturesSearchPath = IO::Path(table["builtin"]);
            }
            
            return GameConfig::TextureConfig(type, property, IO::Path(palette), builtinTexturesSearchPath);
        }

        Model::GameConfig::EntityConfig GameConfigParser::parseEntityConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            Path::List defFilePaths;
            expectTableEntry("definitions", ConfigEntry::Type_Value | ConfigEntry::Type_List, table);
            if (table["definitions"].type() == ConfigEntry::Type_Value) {
                const String pathStr = table["definitions"];
                defFilePaths.push_back(Path(pathStr));
            } else {
                const StringList pathStrs = parseList(table["definitions"]);
                for (size_t i = 0; i < pathStrs.size(); ++i)
                    defFilePaths.push_back(Path(pathStrs[i]));
            }
            
            expectTableEntry("modelformats", ConfigEntry::Type_List, table);
            const StringSet modelFormats = parseSet(table["modelformats"]);
            
            expectTableEntry("defaultcolor", ConfigEntry::Type_Value, table);
            const Color defaultColor(table["defaultcolor"]);
            
            return GameConfig::EntityConfig(defFilePaths, modelFormats, defaultColor);
        }

        Model::GameConfig::FaceAttribsConfig GameConfigParser::parseFaceAttribsConfig(const ConfigTable& table) const {
            using Model::GameConfig;
            
            GameConfig::FlagConfigList surfaceFlags;
            if (table.contains("surfaceflags")) {
                expectTableEntry("surfaceflags", ConfigEntry::Type_List, table);
                surfaceFlags = parseFlagConfig(table["surfaceflags"]);
            }
            
            GameConfig::FlagConfigList contentFlags;
            if (table.contains("contentflags")) {
                expectTableEntry("contentflags", ConfigEntry::Type_List, table);
                contentFlags = parseFlagConfig(table["contentflags"]);
            }
            
            return GameConfig::FaceAttribsConfig(surfaceFlags, contentFlags);
        }
        
        Model::GameConfig::FlagConfigList GameConfigParser::parseFlagConfig(const ConfigList& list) const {
            using Model::GameConfig;
            
            GameConfig::FlagConfigList flags;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::Type_Table, entry);
                const ConfigTable& table = static_cast<const ConfigTable&>(entry);
                
                expectTableEntry("name", ConfigEntry::Type_Value, table);
                const String name = table["name"];
                
                String description;
                if (table.contains("description")) {
                    expectTableEntry("description", ConfigEntry::Type_Value, table);
                    description = table["description"];
                }
                
                flags.push_back(GameConfig::FlagConfig(name, description));
            }
            
            return flags;
        }

        StringSet GameConfigParser::parseSet(const ConfigList& list) const {
            StringSet result;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::Type_Value, entry);
                result.insert(static_cast<const String&>(entry));
            }
            return result;
        }

        StringList GameConfigParser::parseList(const ConfigList& list) const {
            StringList result;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::Type_Value, entry);
                result.push_back(static_cast<const String&>(entry));
            }
            return result;
        }
        
        void GameConfigParser::expectEntry(const int typeMask, const ConfigEntry& entry) const {
            if ((typeMask & entry.type()) == 0)
                throw ParserException("Expected " + typeNames(typeMask) + ", but got " + typeNames(entry.type()));
        }

        void GameConfigParser::expectTableEntry(const String& key, const int typeMask, const ConfigTable& parent) const {
            if (!parent.contains(key))
                throw ParserException("Expected table entry '" + key + "' with type " + typeNames(typeMask));
            if ((parent[key].type() & typeMask) == 0)
                throw ParserException("Expected table entry '" + key + "' with type " + typeNames(typeMask) + ", but got table entry with type '" + typeNames(parent[key].type()) + "'");
        }
        
        String GameConfigParser::typeNames(const int typeMask) const {
            StringList result;
            if ((typeMask & ConfigEntry::Type_Value) != 0)
                result.push_back("value");
            if ((typeMask & ConfigEntry::Type_List) != 0)
                result.push_back("list");
            if ((typeMask & ConfigEntry::Type_Table) != 0)
                result.push_back("table");
            
            if (result.empty())
                return "none";
            if (result.size() == 1)
                return result.front();
            return StringUtils::join(result, ", ", ", or ", " or ");
        }
    }
}
