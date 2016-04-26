/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CompilationConfigParser.h"

#include "CollectionUtils.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        CompilationConfigParser::CompilationConfigParser(const char* begin, const char* end, const Path& path) :
        ConfigParserBase(begin, end, path) {}
        
        CompilationConfigParser::CompilationConfigParser(const String& str, const Path& path) :
        ConfigParserBase(str, path) {}
        
        Model::CompilationConfig CompilationConfigParser::parse() {
            ConfigPtr root = parseConfigFile();
            if (root.get() == NULL)
                throw ParserException("Empty compilation config");
            
            expectEntry(ConfigEntry::Type_Table, *root);
            const ConfigTable& rootTable = *root;
            
            expectTableEntries(rootTable,
                               StringUtils::makeSet(2, "version", "profiles"),
                               StringSet());
            
            expectTableEntry("version", ConfigEntry::Type_Value, rootTable);
            const String version = rootTable["version"];
            
            expectTableEntry("profiles", ConfigEntry::Type_List, rootTable);
            const Model::CompilationProfile::List profiles = parseProfiles(rootTable["profiles"]);
            
            return Model::CompilationConfig(profiles);
        }

        Model::CompilationProfile::List CompilationConfigParser::parseProfiles(const ConfigList& list) const {
            Model::CompilationProfile::List result;
            
            try {
                for (size_t i = 0; i < list.count(); ++i) {
                    expectListEntry(i, ConfigEntry::Type_Table, list);
                    result.push_back(parseProfile(list[i]));
                }
                return result;
            } catch (...) {
                VectorUtils::clearAndDelete(result);
                throw;
            }
        }

        Model::CompilationProfile* CompilationConfigParser::parseProfile(const ConfigTable& table) const {
            expectTableEntries(table,
                               StringUtils::makeSet(3, "name", "workdir", "tasks"),
                               StringSet());

            expectTableEntry("name", ConfigEntry::Type_Value, table);
            const String name = table["name"];
            
            expectTableEntry("workdir", ConfigEntry::Type_Value, table);
            const String workdir = table["workdir"];
            
            expectTableEntry("tasks", ConfigEntry::Type_List, table);
            const Model::CompilationTask::List tasks = parseTasks(table["tasks"]);
            
            return new Model::CompilationProfile(name, workdir, tasks);
        }

        Model::CompilationTask::List CompilationConfigParser::parseTasks(const ConfigList& list) const {
            Model::CompilationTask::List result;
            
            try {
                for (size_t i = 0; i < list.count(); ++i) {
                    expectListEntry(i, ConfigEntry::Type_Table, list);
                    result.push_back(parseTask(list[i]));
                }
                return result;
            } catch (...) {
                VectorUtils::clearAndDelete(result);
                throw;
            }
        }
        
        Model::CompilationTask* CompilationConfigParser::parseTask(const ConfigTable& table) const {
            expectTableEntry("type", ConfigEntry::Type_Value, table);
            const String type = table["type"];
            
            if (type == "export")
                return parseExportTask(table);
            else if (type == "copy")
                return parseCopyTask(table);
            else if (type == "tool")
                return parseToolTask(table);
            else
                throw ParserException("Unknown compilation task type '" + type + "'");
        }
        
        Model::CompilationTask* CompilationConfigParser::parseExportTask(const ConfigTable& table) const {
            expectTableEntries(table,
                               StringUtils::makeSet(2, "type", "target"),
                               StringSet());
            
            expectTableEntry("target", ConfigEntry::Type_Value, table);
            const String target = table["target"];
            
            return new Model::CompilationExportMap(target);
        }

        Model::CompilationTask* CompilationConfigParser::parseCopyTask(const ConfigTable& table) const {
            expectTableEntries(table,
                               StringUtils::makeSet(3, "type", "source", "target"),
                               StringSet());
            
            expectTableEntry("source", ConfigEntry::Type_Value, table);
            const String source = table["source"];

            expectTableEntry("target", ConfigEntry::Type_Value, table);
            const String target = table["target"];
            
            return new Model::CompilationCopyFiles(source, target);
        }
        
        Model::CompilationTask* CompilationConfigParser::parseToolTask(const ConfigTable& table) const {
            expectTableEntries(table,
                               StringUtils::makeSet(3, "type", "tool", "parameters"),
                               StringSet());
            
            expectTableEntry("tool", ConfigEntry::Type_Value, table);
            const String tool = table["tool"];

            expectTableEntry("parameters", ConfigEntry::Type_Value, table);
            const String parameters = table["parameters"];
            
            return new Model::CompilationRunTool(tool, parameters);
        }
    }
}
