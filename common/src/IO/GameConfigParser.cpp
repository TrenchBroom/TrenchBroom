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
          
            const auto root = parseConfigFile().evaluate(EL::EvaluationContext());
            expectType(root, EL::Type_Map);
            
            expectStructure(root,
                            "["
                            "{'version': 'Number', 'name': 'String', 'fileformats': 'Array', 'filesystem': 'Map', 'textures': 'Map', 'entities': 'Map'},"
                            "{'icon': 'String', 'experimental': 'Boolean', 'faceattribs': 'Map', 'brushtypes': 'Array', 'tags': 'Map'}"
                            "]");

            const auto version = root["version"].numberValue();
            unused(version);
            assert(version == 2.0);

            const auto& name = root["name"].stringValue();
            const auto icon = Path(root["icon"].stringValue());
            const auto experimental = root["experimental"].booleanValue();

            const auto mapFormatConfigs = parseMapFormatConfigs(root["fileformats"]);
            const auto fileSystemConfig = parseFileSystemConfig(root["filesystem"]);
            const auto textureConfig = parseTextureConfig(root["textures"]);
            const auto entityConfig = parseEntityConfig(root["entities"]);
            const auto faceAttribsConfig = parseFaceAttribsConfig(root["faceattribs"]);
                  auto brushContentTypes = parseBrushContentTypes(root["brushtypes"], faceAttribsConfig);
                  auto tags = parseTags(root["tags"]);
            
            return GameConfig(name, m_path, icon, experimental, mapFormatConfigs, fileSystemConfig, textureConfig, entityConfig, faceAttribsConfig, std::move(brushContentTypes));
        }

        Model::GameConfig::MapFormatConfig::List GameConfigParser::parseMapFormatConfigs(const EL::Value& value) const {
            using Model::GameConfig;

            expectType(value, EL::typeForName("Array"));

            GameConfig::MapFormatConfig::List result;
            for (size_t i = 0; i < value.length(); ++i) {
                expectStructure(
                    value[i],
                    "["
                    "{'format': 'String'},"
                    "{'initialmap': 'String'}"
                    "]");

                const String& format = value[i]["format"].stringValue();
                const String& initialMap = value[i]["initialmap"].stringValue();

                result.emplace_back(format, IO::Path(initialMap));
            }

            return result;
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

            expectMapEntry(value, "format", EL::typeForName("String"));
            const auto formatValue = value["format"];
            expectType(formatValue, EL::typeForName("String"));

            if (value["extension"] != EL::Value::Null) {
                const auto extensionValue = value["extension"];
                expectType(extensionValue, EL::typeForName("String"));
                const auto& extension = value["extension"].stringValue();
                const auto& format = formatValue.stringValue();

                return GameConfig::PackageFormatConfig(extension, format);
            } else if (value["extensions"] != EL::Value::Null) {
                const auto extensionsValue = value["extensions"];
                expectType(extensionsValue, EL::typeForName("Array"));
                const auto extensions = extensionsValue.asStringList();
                const auto& format = formatValue.stringValue();

                return GameConfig::PackageFormatConfig(extensions, format);
            }
            throw ParserException(value.line(), value.column(), "Expected map entry 'extension' of type 'String' or 'extensions' of type 'Array'");
        }

        Model::GameConfig::TextureConfig GameConfigParser::parseTextureConfig(const EL::Value& value) const {
            using Model::GameConfig;
            
            expectStructure(value,
                            "["
                            "{'package': 'Map', 'format': 'Map'},"
                            "{'attribute': 'String', 'palette': 'String'}"
                            "]");

            const GameConfig::TexturePackageConfig packageConfig = parseTexturePackageConfig(value["package"]);
            const GameConfig::PackageFormatConfig formatConfig = parsePackageFormatConfig(value["format"]);
            const Path palette(value["palette"].stringValue());
            const String& attribute = value["attribute"].stringValue();

            return GameConfig::TextureConfig(packageConfig, formatConfig, palette, attribute);
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
            
            if (value.null()) {
                return Model::BrushContentType::List();
            }

            Model::BrushContentType::List contentTypes;
            for (size_t i = 0; i < value.length(); ++i) {
                const auto& entry = value[i];
                
                expectStructure(entry, "[ {'name': 'String', 'match': 'String'}, {'attribs': 'Array', 'pattern': 'String', 'flags': 'Array', 'ignore': 'Array' } ]");

                const auto& name = entry["name"].stringValue();
                const auto transparent = entry["attribs"].asStringSet().count("transparent") > 0;
                const auto& match = entry["match"].stringValue();
                const auto& ignoreTexture = entry["ignore"].asStringList();

                const Model::BrushContentType::FlagType flag = 1 << i;
                
                if (match == "texture") {
                    expectMapEntry(entry, "pattern", EL::Type_String);
                    const auto& pattern = entry["pattern"].stringValue();
                    auto evaluator = Model::BrushContentTypeEvaluator::textureNameEvaluator(pattern, ignoreTexture);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, std::move(evaluator)));
                } else if (match == "surfaceparm") {
                    expectMapEntry(entry, "pattern", EL::Type_String);
                    const auto& pattern = entry["pattern"].stringValue();
                    auto evaluator = Model::BrushContentTypeEvaluator::shaderSurfaceParmsEvaluator(pattern, ignoreTexture);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, std::move(evaluator)));
                } else if (match == "contentflag") {
                    expectMapEntry(entry, "flags", EL::Type_Array);
                    const StringSet flagSet = entry["flags"].asStringSet();
                    int flagValue = 0;

                    for (const String &currentName : flagSet) {
                        const int currentValue = faceAttribsConfig.contentFlags.flagValue(currentName);
                        flagValue |= currentValue;
                    }

                    auto evaluator = Model::BrushContentTypeEvaluator::contentFlagsEvaluator(flagValue, ignoreTexture);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, std::move(evaluator)));
                } else if (match == "surfaceflag") {
                    expectMapEntry(entry, "flags", EL::Type_Array);
                    const StringSet flagSet = entry["flags"].asStringSet();
                    int flagValue = 0;

                    for (const String &currentName : flagSet) {
                        const int currentValue = faceAttribsConfig.contentFlags.flagValue(currentName);
                        flagValue |= currentValue;
                    }

                    auto evaluator = Model::BrushContentTypeEvaluator::surfaceFlagsEvaluator(flagValue, ignoreTexture);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, std::move(evaluator)));
                } else if (match == "classname") {
                    const String& pattern = entry["pattern"].stringValue();
                    auto evaluator = Model::BrushContentTypeEvaluator::entityClassnameEvaluator(pattern);
                    contentTypes.push_back(Model::BrushContentType(name, transparent, flag, std::move(evaluator)));
                } else {
                    throw ParserException(entry.line(), entry.column(), "Unexpected brush content type '" + match + "'");
                }
            }
            return contentTypes;
        }

        std::vector<Model::SmartTag> GameConfigParser::parseTags(const EL::Value& value) const {
            std::vector<Model::SmartTag> result{};
            if (value.null()) {
                return result;
            }

            expectStructure(value,
                            "["
                            "{},"
                            "{'brush': 'Array', 'face': 'Array'}"
                            "]");

            parseBrushTags(value["brush"], result);
            parseFaceTags(value["face"], result);
            return result;
        }

        void GameConfigParser::parseBrushTags(const EL::Value& value, std::vector<Model::SmartTag>& result) const {
            if (value.null()) {
                return;
            }

        }

        void GameConfigParser::parseFaceTags(const EL::Value& value, std::vector<Model::SmartTag>& result) const {
            if (value.null()) {
                return;
            }
        }
    }
}
