/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "GameEngineConfigParser.h"

#include "Macros.h"
#include "CollectionUtils.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        GameEngineConfigParser::GameEngineConfigParser(const char* begin, const char* end, const Path& path) :
        ConfigParserBase(begin, end, path) {}
        
        GameEngineConfigParser::GameEngineConfigParser(const String& str, const Path& path) :
        ConfigParserBase(str, path) {}
        
        Model::GameEngineConfig GameEngineConfigParser::parse() {
            const EL::Value root = parseConfigFile().evaluate(EL::EvaluationContext());

            expectStructure(root, "[ {'version': 'Number', 'profiles': 'Array'}, {} ]");

            const EL::NumberType version = root["version"].numberValue();
            unused(version);
            assert(version == 1.0);

            const Model::GameEngineProfile::List profiles = parseProfiles(root["profiles"]);
            
            return Model::GameEngineConfig(profiles);
        }

        Model::GameEngineProfile::List GameEngineConfigParser::parseProfiles(const EL::Value& value) const {
            Model::GameEngineProfile::List result;
            
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
        
        Model::GameEngineProfile* GameEngineConfigParser::parseProfile(const EL::Value& value) const {
            expectStructure(value, "[ {'name': 'String', 'path': 'String'}, { 'parameters': 'String' } ]");

            const String& name = value["name"].stringValue();
            const Path path = Path(value["path"].stringValue());
            const String& parameterSpec = value["parameters"].stringValue();
            
            return new Model::GameEngineProfile(name, path, parameterSpec);
        }
    }
}
