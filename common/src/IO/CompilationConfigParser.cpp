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

#include "Macros.h"
#include "CollectionUtils.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        CompilationConfigParser::CompilationConfigParser(const char* begin, const char* end, const Path& path) :
        ConfigParserBase(begin, end, path) {}
        
        CompilationConfigParser::CompilationConfigParser(const String& str, const Path& path) :
        ConfigParserBase(str, path) {}
        
        Model::CompilationConfig CompilationConfigParser::parse() {
            const EL::Value root = parseConfigFile().evaluate(EL::EvaluationContext());
            expectType(root, EL::Type_Map);
            
            expectStructure(root, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");
            
            const EL::NumberType version = root["version"].numberValue();
            unused(version);
            assert(version == 1.0);
            
            const Model::CompilationProfile::Array profiles = parseProfiles(root["profiles"]);

            return Model::CompilationConfig(profiles);
        }

        Model::CompilationProfile::Array CompilationConfigParser::parseProfiles(const EL::Value& value) const {
            Model::CompilationProfile::Array result;
            
            try {
                for (size_t i = 0; i < value.length(); ++i) {
                    result.push_back(parseProfile(value[i]));
                }
                return result;
            } catch (...) {
                VectorUtils::clearAndDelete(result);
                throw;
            }
        }

        Model::CompilationProfile* CompilationConfigParser::parseProfile(const EL::Value& value) const {
            expectStructure(value, "[ {'name': 'String', 'workdir': 'String', 'tasks': 'Array'}, {} ]");

            const String& name = value["name"].stringValue();
            const String& workdir = value["workdir"].stringValue();
            const Model::CompilationTask::Array tasks = parseTasks(value["tasks"]);
            
            return new Model::CompilationProfile(name, workdir, tasks);
        }

        Model::CompilationTask::Array CompilationConfigParser::parseTasks(const EL::Value& value) const {
            Model::CompilationTask::Array result;
            
            try {
                for (size_t i = 0; i < value.length(); ++i) {
                    result.push_back(parseTask(value[i]));
                }
                return result;
            } catch (...) {
                VectorUtils::clearAndDelete(result);
                throw;
            }
        }
        
        Model::CompilationTask* CompilationConfigParser::parseTask(const EL::Value& value) const {
            expectMapEntry(value, "type", EL::Type_String);
            const String& type = value["type"].stringValue();
            
            if (type == "export")
                return parseExportTask(value);
            else if (type == "copy")
                return parseCopyTask(value);
            else if (type == "tool")
                return parseToolTask(value);
            else
                throw ParserException("Unknown compilation task type '" + type + "'");
        }
        
        Model::CompilationTask* CompilationConfigParser::parseExportTask(const EL::Value& value) const {
            expectStructure(value, "[ {'type': 'String', 'target': 'String'}, {} ]");
            const String& target = value["target"].stringValue();
            return new Model::CompilationExportMap(target);
        }

        Model::CompilationTask* CompilationConfigParser::parseCopyTask(const EL::Value& value) const {
            expectStructure(value, "[ {'type': 'String', 'source': 'String', 'target': 'String'}, {} ]");

            const String& source = value["source"].stringValue();
            const String& target = value["target"].stringValue();
            
            return new Model::CompilationCopyFiles(source, target);
        }
        
        Model::CompilationTask* CompilationConfigParser::parseToolTask(const EL::Value& value) const {
            expectStructure(value, "[ {'type': 'String', 'tool': 'String', 'parameters': 'String'}, {} ]");

            const String& tool = value["tool"].stringValue();
            const String& parameters = value["parameters"].stringValue();
            
            return new Model::CompilationRunTool(tool, parameters);
        }
    }
}
