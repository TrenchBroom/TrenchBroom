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

#include "GameConfigParser.h"

#include "Exceptions.h"
#include "Model/BrushContentTypeEvaluator.h"

namespace TrenchBroom {
    namespace IO {
        GameConfigParser::GameConfigParser(const char* begin, const char* end, const Path& path) :
        ConfigParserBase(begin, end, path) {}
        
        GameConfigParser::GameConfigParser(const String& str, const Path& path) :
        ConfigParserBase(str, path) {}
        
        Model::GameConfig GameConfigParser::parse() {
            using Model::GameConfig;
          
            const EL::Value root = parseConfigFile().evaluate(EL::EvaluationContext());
            expectType(root, EL::Type_Map);
            
            expectStructure(root,
                            "["
                            "{'version': 'Number', 'name': 'String', 'fileformats': 'Array', 'filesystem': 'Map', 'textures': 'Map', 'entities': 'Map'},"
                            "{'icon': 'String', 'faceattribs': 'Map', 'brushtypes': 'Array'}"
                            "]");

            const EL::NumberType version = root["version"].numberValue();
            unused(version);
            assert(version == 1.0);

            const String& name = root["name"].stringValue();
            const Path icon(root["icon"].stringValue());
            
            const StringList fileFormats = root["fileformats"].asStringList();
            const GameConfig::FileSystemConfig fileSystemConfig = parseFileSystemConfig(root["filesystem"]);
            const GameConfig::TextureConfig textureConfig = parseTextureConfig(root["textures"]);
            const GameConfig::EntityConfig entityConfig = parseEntityConfig(root["entities"]);
            const GameConfig::FaceAttribsConfig faceAttribsConfig = parseFaceAttribsConfig(root["faceattribs"]);
            const Model::BrushContentType::List brushContentTypes = parseBrushContentTypes(root["brushtypes"], faceAttribsConfig);
            
            return GameConfig(name, m_path, icon, fileFormats, fileSystemConfig, textureConfig, entityConfig, faceAttribsConfig, brushContentTypes);
        }

        Model::GameConfig::FileSystemConfig GameConfigParser::parseFileSystemConfig(const EL::Value& value) const {
            using Model::GameConfig;

            expectStructure(value,
                            "["
                            "{'searchpath': 'String', 'packageformat': 'Map'},"
                            "{}"
                            "]");
            

            const String& searchPath = value["searchpath"].stringValue();
            const GameConfig::PackageFormatConfig packageFormatConfig = parsePackageFormatConfig(value["packageformat"]);
            
            return GameConfig::FileSystemConfig(Path(searchPath), packageFormatConfig);
        }

        Model::GameConfig::PackageFormatConfig GameConfigParser::parsePackageFormatConfig(const EL::Value& value) const {
            using Model::GameConfig;
            
            expectStructure(value,
                            "["
                            "{'extension': 'String', 'format': 'String'},"
                            "{}"
                            "]");

            const String& extension = value["extension"].stringValue();
            const String& format = value["format"].stringValue();
            
            return GameConfig::PackageFormatConfig(extension, format);
        }

        Model::GameConfig::TextureConfig GameConfigParser::parseTextureConfig(const EL::Value& value) const {
            using Model::GameConfig;
            
            expectStructure(value,
                            "["
                            "{'package': 'Map', 'format': 'Map'},"
                            "{'attribute': 'String', 'palette': 'String', 'palettefallback': 'String'}"
                            "]");

            const GameConfig::TexturePackageConfig packageConfig = parseTexturePackageConfig(value["package"]);
            const GameConfig::PackageFormatConfig formatConfig = parsePackageFormatConfig(value["format"]);
            const Path palette(value["palette"].stringValue());
            const Path palettefallback(value["palettefallback"].stringValue());
            const String& attribute = value["attribute"].stringValue();

            return GameConfig::TextureConfig(packageConfig, formatConfig, palette, palettefallback, attribute);
        }

        Model::GameConfig::TexturePackageConfig GameConfigParser::parseTexturePackageConfig(const EL::Value& value) const {
            using Model::GameConfig;

            expectStructure(value,
                            "["
                            "{'type': 'String'},"
                            "{'root': 'String', 'format': 'Map'}"
                            "]");

            const String& typeStr = value["type"].stringValue();
            if (typeStr == "file") {
                expectMapEntry(value, "format", EL::Type_Map);
                const GameConfig::PackageFormatConfig formatConfig = parsePackageFormatConfig(value["format"]);
                return GameConfig::TexturePackageConfig(formatConfig);
            } else if (typeStr == "directory") {
                expectMapEntry(value, "root", EL::Type_String);
                const Path root(value["root"].stringValue());
                return GameConfig::TexturePackageConfig(root);
            } else {
                throw ParserException(value.line(), value.column(), "Unexpected texture package type '" + typeStr + "'");
            }
        }

        Model::GameConfig::EntityConfig GameConfigParser::parseEntityConfig(const EL::Value& value) const {
            using Model::GameConfig;
            
            expectStructure(value,
                            "["
                            "{'definitions': 'Array', 'modelformats': 'Array', 'defaultcolor': 'String'},"
                            "{}"
                            "]");

            const Path::List defFilePaths = Path::asPaths(value["definitions"].asStringList());
            const StringSet modelFormats = value["modelformats"].asStringSet();
            const Color defaultColor = Color::parse(value["defaultcolor"].stringValue());
            
            return GameConfig::EntityConfig(defFilePaths, modelFormats, defaultColor);
        }

        Model::GameConfig::FaceAttribsConfig GameConfigParser::parseFaceAttribsConfig(const EL::Value& value) const {
            using Model::GameConfig;
            
            if (value.null())
                return Model::GameConfig::FaceAttribsConfig();
            
            expectStructure(value,
                            "["
                            "{'surfaceflags': 'Array', 'contentflags': 'Array'},"
                            "{}"
                            "]");

            const GameConfig::FlagConfigList surfaceFlags = parseFlagConfig(value["surfaceflags"]);
            const GameConfig::FlagConfigList contentFlags = parseFlagConfig(value["contentflags"]);
            
            return GameConfig::FaceAttribsConfig(surfaceFlags, contentFlags);
        }
        
        Model::GameConfig::FlagConfigList GameConfigParser::parseFlagConfig(const EL::Value& value) const {
            using Model::GameConfig;

            if (value.null())
                return GameConfig::FlagConfigList(0);
            
            GameConfig::FlagConfigList flags;
            for (size_t i = 0; i < value.length(); ++i) {
                const EL::Value& entry = value[i];
                
                expectStructure(entry, "[ {'name': 'String'}, {'description': 'String'} ]");
                
                const String& name = entry["name"].stringValue();
                const String& description = entry["description"].stringValue();
                
                flags.push_back(GameConfig::FlagConfig(name, description));
            }
            
            return flags;
        }

        Model::BrushContentType::List GameConfigParser::parseBrushContentTypes(const EL::Value& value, const Model::GameConfig::FaceAttribsConfig& faceAttribsConfig) const {
            using Model::GameConfig;
            
            if (value.null())
                return Model::BrushContentType::List();
            
            Model::BrushContentType::List contentTypes;
            for (size_t i = 0; i < value.length(); ++i) {
                const EL::Value& entry = value[i];
                
                expectStructure(entry, "[ {'name': 'String', 'match': 'String'}, {'attribs': 'Array', 'pattern': 'String', 'flags': 'Array' } ]");

                const String& name = entry["name"].stringValue();
                const bool transparent = entry["attribs"].asStringSet().count("transparent") > 0;
                const String& match = entry["match"].stringValue();

                const Model::BrushContentType::FlagType flag = 1 << i;
                
                if (match == "texture") {
                    expectMapEntry(entry, "pattern", EL::Type_String);
                    const String& pattern = entry["pattern"].stringValue();
                    Model::BrushContentTypeEvaluator* evaluator = Model::BrushContentTypeEvaluator::textureNameEvaluator(pattern);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, evaluator));
                } else if (match == "contentflag") {
                    expectMapEntry(entry, "flags", EL::Type_Array);
                    const StringSet flagSet = entry["flags"].asStringSet();
                    int flagValue = 0;

                    for (const String& currentName : flagSet) {
                        const int currentValue = faceAttribsConfig.contentFlags.flagValue(currentName);
                        flagValue |= currentValue;
                    }
                    
                    Model::BrushContentTypeEvaluator* evaluator = Model::BrushContentTypeEvaluator::contentFlagsEvaluator(flagValue);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, evaluator));
                } else if (match == "classname") {
                    const String& pattern = entry["pattern"].stringValue();
                    Model::BrushContentTypeEvaluator* evaluator = Model::BrushContentTypeEvaluator::entityClassnameEvaluator(pattern);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, evaluator));
                } else {
                    throw ParserException(entry.line(), entry.column(), "Unexpected brush content type '" + match + "'");
                }
            }
            return contentTypes;
        }
    }
}
