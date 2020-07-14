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
#include "EL/EvaluationContext.h"
#include "EL/Expression.h"
#include "EL/Value.h"
#include "FloatType.h"
#include "Model/GameConfig.h"
#include "Model/Tag.h"
#include "Model/TagAttribute.h"
#include "Model/TagMatcher.h"

#include <vecmath/vec_io.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        GameConfigParser::GameConfigParser(const char* begin, const char* end, const Path& path) :
        ConfigParserBase(begin, end, path),
        m_version(0) {}

        GameConfigParser::GameConfigParser(const std::string& str, const Path& path) :
        ConfigParserBase(str, path),
        m_version(0) {}

        static void checkVersion(const EL::Value& version) {
            const std::vector<EL::IntegerType> validVsns({3, 4});
            const bool isNumVersion = version.convertibleTo(EL::ValueType::Number);
            const bool isValidVersion = isNumVersion && (std::find(validVsns.begin(), validVsns.end(), version.integerValue()) != validVsns.end());
            if (!isValidVersion) {
                const std::string versionStr(version.convertTo(EL::ValueType::String).stringValue());
                const std::string validVsnsStr(kdl::str_join(validVsns, ", "));
                throw ParserException(version.line(), version.column(), " Unsupported game configuration version " + versionStr + "; valid versions are: " + validVsnsStr);
            }
        }

        Model::GameConfig GameConfigParser::parse() {
            using Model::GameConfig;

            const auto root = parseConfigFile().evaluate(EL::EvaluationContext());
            expectType(root, EL::ValueType::Map);

            const EL::Value version = root["version"];
            checkVersion(version);
            m_version = version.integerValue();

            expectStructure(root,
                            "["
                            "{'version': 'Number', 'name': 'String', 'fileformats': 'Array', 'filesystem': 'Map', 'textures': 'Map', 'entities': 'Map'},"
                            "{'icon': 'String', 'experimental': 'Boolean', 'faceattribs': 'Map', 'tags': 'Map', 'softMapBounds': 'String'}"
                            "]");

            auto name = root["name"].stringValue();
            auto icon = Path(root["icon"].stringValue());
            auto experimental = root["experimental"].booleanValue();

            auto mapFormatConfigs = parseMapFormatConfigs(root["fileformats"]);
            auto fileSystemConfig = parseFileSystemConfig(root["filesystem"]);
            auto textureConfig = parseTextureConfig(root["textures"]);
            auto entityConfig = parseEntityConfig(root["entities"]);
            auto faceAttribsConfig = parseFaceAttribsConfig(root["faceattribs"]);
            auto tags = parseTags(root["tags"], faceAttribsConfig);
            auto softMapBounds = parseSoftMapBounds(root["softMapBounds"]);

            return GameConfig(
                std::move(name),
                m_path,
                std::move(icon),
                experimental,
                std::move(mapFormatConfigs),
                std::move(fileSystemConfig),
                std::move(textureConfig),
                std::move(entityConfig),
                std::move(faceAttribsConfig),
                std::move(tags),
                std::move(softMapBounds));
        }

        std::vector<Model::MapFormatConfig> GameConfigParser::parseMapFormatConfigs(const EL::Value& value) const {
            expectType(value, EL::typeForName("Array"));

            std::vector<Model::MapFormatConfig> result;
            for (size_t i = 0; i < value.length(); ++i) {
                expectStructure(
                    value[i],
                    "["
                    "{'format': 'String'},"
                    "{'initialmap': 'String'}"
                    "]");

                const std::string format = value[i]["format"].stringValue();
                const std::string initialMap = value[i]["initialmap"].stringValue();

                result.emplace_back(format, IO::Path(initialMap));
            }

            return result;
        }

        Model::FileSystemConfig GameConfigParser::parseFileSystemConfig(const EL::Value& value) const {
            expectStructure(value,
                            "["
                            "{'searchpath': 'String', 'packageformat': 'Map'},"
                            "{}"
                            "]");


            const std::string searchPath = value["searchpath"].stringValue();
            const Model::PackageFormatConfig packageFormatConfig = parsePackageFormatConfig(value["packageformat"]);

            return Model::FileSystemConfig(Path(searchPath), packageFormatConfig);
        }

        Model::PackageFormatConfig GameConfigParser::parsePackageFormatConfig(const EL::Value& value) const {
            expectMapEntry(value, "format", EL::typeForName("String"));
            const auto formatValue = value["format"];
            expectType(formatValue, EL::typeForName("String"));

            if (value["extension"] != EL::Value::Null) {
                const auto extensionValue = value["extension"];
                expectType(extensionValue, EL::typeForName("String"));
                const auto extension = value["extension"].stringValue();
                const auto format = formatValue.stringValue();

                return Model::PackageFormatConfig(extension, format);
            } else if (value["extensions"] != EL::Value::Null) {
                const auto extensionsValue = value["extensions"];
                expectType(extensionsValue, EL::typeForName("Array"));
                const auto extensions = extensionsValue.asStringList();
                const auto format = formatValue.stringValue();

                return Model::PackageFormatConfig(extensions, format);
            }
            throw ParserException(value.line(), value.column(), "Expected map entry 'extension' of type 'String' or 'extensions' of type 'Array'");
        }

        Model::TextureConfig GameConfigParser::parseTextureConfig(const EL::Value& value) const {
            expectStructure(value,
                            "["
                            "{'package': 'Map', 'format': 'Map'},"
                            "{'attribute': 'String', 'palette': 'String', 'shaderSearchPath': 'String', 'excludes': 'Array'}"
                            "]");

            const Model::TexturePackageConfig packageConfig = parseTexturePackageConfig(value["package"]);
            const Model::PackageFormatConfig formatConfig = parsePackageFormatConfig(value["format"]);
            const Path palette(value["palette"].stringValue());
            const std::string attribute = value["attribute"].stringValue();
            const Path shaderSearchPath(value["shaderSearchPath"].stringValue());
            const std::vector<std::string> excludes = std::vector<std::string>(value["excludes"].asStringList());

            return Model::TextureConfig(packageConfig, formatConfig, palette, attribute, shaderSearchPath, excludes);
        }

        Model::TexturePackageConfig GameConfigParser::parseTexturePackageConfig(const EL::Value& value) const {
            expectStructure(value,
                            "["
                            "{'type': 'String'},"
                            "{'root': 'String', 'format': 'Map'}"
                            "]");

            const std::string typeStr = value["type"].stringValue();
            if (typeStr == "file") {
                expectMapEntry(value, "format", EL::ValueType::Map);
                const Model::PackageFormatConfig formatConfig = parsePackageFormatConfig(value["format"]);
                return Model::TexturePackageConfig(formatConfig);
            } else if (typeStr == "directory") {
                expectMapEntry(value, "root", EL::ValueType::String);
                const Path root(value["root"].stringValue());
                return Model::TexturePackageConfig(root);
            } else {
                throw ParserException(value.line(), value.column(), "Unexpected texture package type '" + typeStr + "'");
            }
        }

        Model::EntityConfig GameConfigParser::parseEntityConfig(const EL::Value& value) const {
            expectStructure(value,
                            "["
                            "{'definitions': 'Array', 'modelformats': 'Array', 'defaultcolor': 'String'},"
                            "{}"
                            "]");

            const std::vector<Path> defFilePaths = Path::asPaths(value["definitions"].asStringList());
            const std::vector<std::string> modelFormats = value["modelformats"].asStringSet();
            const Color defaultColor = Color::parse(value["defaultcolor"].stringValue());

            return Model::EntityConfig(defFilePaths, modelFormats, defaultColor);
        }

        Model::FaceAttribsConfig GameConfigParser::parseFaceAttribsConfig(const EL::Value& value) const {
            if (value.null())
                return Model::FaceAttribsConfig();

            expectStructure(value,
                            "["
                            "{'surfaceflags': 'Array', 'contentflags': 'Array'},"
                            "{'defaults': 'Map'}"
                            "]");

            const Model::FlagsConfig surfaceFlags = parseFlagsConfig(value["surfaceflags"]);
            const Model::FlagsConfig contentFlags = parseFlagsConfig(value["contentflags"]);
            const Model::BrushFaceAttributes defaults = parseFaceAttribsDefaults(value["defaults"], surfaceFlags, contentFlags);

            return Model::FaceAttribsConfig(surfaceFlags, contentFlags, defaults);
        }

        Model::FlagsConfig GameConfigParser::parseFlagsConfig(const EL::Value& value) const {
            using Model::GameConfig;

            if (value.null()) {
                return {};
            }

            std::vector<Model::FlagConfig> flags;
            for (size_t i = 0; i < value.length(); ++i) {
                parseFlag(value[i], i, flags);
            }

            return Model::FlagsConfig(flags);
        }

        void GameConfigParser::parseFlag(const EL::Value& value, const size_t index, std::vector<Model::FlagConfig>& flags) const {
            bool unused;
            if (m_version == 3) {
                unused = false;
                expectStructure(value,
                                "["
                                "{'name': 'String'},"
                                "{'description': 'String'}"
                                "]");
            } else {
                unused = value["unused"].booleanValue();
                if (unused) {
                    expectStructure(value,
                                    "["
                                    "{},"
                                    "{'name': 'String', 'description': 'String', 'unused': 'Boolean'}"
                                    "]");
                } else {
                    expectStructure(value,
                                    "["
                                    "{'name': 'String'},"
                                    "{'description': 'String', 'unused': 'Boolean'}"
                                    "]");
                }
            }
            if (!unused) {
                const std::string name = value["name"].stringValue();
                const std::string description = value["description"].stringValue();
                flags.push_back(Model::FlagConfig(name, description, 1 << index));
            }
        }

        Model::BrushFaceAttributes GameConfigParser::parseFaceAttribsDefaults(const EL::Value& value, const Model::FlagsConfig& surfaceFlags, const Model::FlagsConfig& contentFlags) const {
            Model::BrushFaceAttributes defaults(Model::BrushFaceAttributes::NoTextureName);
            if (value.null()) {
                return defaults;
            }

            expectStructure(value,
                            "["
                            "{},"
                            "{'textureName': 'String', 'offset': 'Array', 'scale': 'Array', 'rotation': 'Number', 'surfaceContents': 'Array', 'surfaceFlags': 'Array', 'surfaceValue': 'Number', 'color': 'String'}"
                            "]");

            if (!value["textureName"].null()) {
                defaults = Model::BrushFaceAttributes(value["textureName"].stringValue());
            }
            if (!value["offset"].null() && value["offset"].length() == 2) {
                auto offset = value["offset"];
                defaults.setOffset(vm::vec2f(offset[0].numberValue(), offset[1].numberValue()));
            }
            if (!value["scale"].null() && value["scale"].length() == 2) {
                auto scale = value["scale"];
                defaults.setScale(vm::vec2f(scale[0].numberValue(), scale[1].numberValue()));
            }
            if (!value["rotation"].null()) {
                defaults.setRotation(static_cast<float>(value["rotation"].numberValue()));
            }
            if (!value["surfaceContents"].null()) {
                int defaultSurfaceContents = 0;
                for (size_t i = 0; i < value["surfaceContents"].length(); ++i) {
                    auto name = value["surfaceContents"][i].stringValue();
                    defaultSurfaceContents |= contentFlags.flagValue(name);
                }
                defaults.setSurfaceContents(defaultSurfaceContents);
            }
            if (!value["surfaceFlags"].null()) {
                int defaultSurfaceFlags = 0;
                for (size_t i = 0; i < value["surfaceFlags"].length(); ++i) {
                    auto name = value["surfaceFlags"][i].stringValue();
                    defaultSurfaceFlags |= surfaceFlags.flagValue(name);
                }
                defaults.setSurfaceFlags(defaultSurfaceFlags);
            }
            if (!value["surfaceValue"].null()) {
                defaults.setSurfaceValue(static_cast<float>(value["surfaceValue"].numberValue()));
            }
            if (!value["color"].null()) {
                defaults.setColor(Color::parse(value["color"].stringValue()));
            }

            return defaults;
        }

        std::vector<Model::SmartTag> GameConfigParser::parseTags(const EL::Value& value, const Model::FaceAttribsConfig& faceAttribsConfig) const {
            std::vector<Model::SmartTag> result{};
            if (value.null()) {
                return result;
            }

            expectStructure(value,
                            "["
                            "{},"
                            "{'brush': 'Array', 'brushface': 'Array'}"
                            "]");

            parseBrushTags(value["brush"], result);
            parseFaceTags(value["brushface"], faceAttribsConfig, result);
            return result;
        }

        static void checkTagName(const EL::Value& nameValue, const std::vector<Model::SmartTag>& tags) {
            const auto& name = nameValue.stringValue();
            for (const auto& tag : tags) {
                if (tag.name() == name) {
                    throw ParserException(nameValue.line(), nameValue.column(), "Duplicate tag '" + name + "'");
                }
            }
        }
    
        void GameConfigParser::parseBrushTags(const EL::Value& value, std::vector<Model::SmartTag>& result) const {
            if (value.null()) {
                return;
            }

            for (size_t i = 0; i < value.length(); ++i) {
                const auto entry = value[i];

                expectStructure(entry, "[ {'name': 'String', 'match': 'String'}, {'attribs': 'Array', 'pattern': 'String', 'texture': 'String' } ]");
                checkTagName(entry["name"], result);
                
                auto name = entry["name"].stringValue();
                auto match = entry["match"].stringValue();

                if (match == "classname") {
                    auto pattern = entry["pattern"].stringValue();
                    auto attribs = parseTagAttributes(entry["attribs"]);
                    auto texture = entry["texture"].stringValue();
                    auto matcher = std::make_unique<Model::EntityClassNameTagMatcher>(std::move(pattern), std::move(texture));
                    result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
                } else {
                    throw ParserException(entry.line(), entry.column(), "Unexpected smart tag match type '" + match + "'");
                }
            }
        }

        void GameConfigParser::parseFaceTags(const EL::Value& value, const Model::FaceAttribsConfig& faceAttribsConfig, std::vector<Model::SmartTag>& result) const {
            if (value.null()) {
                return;
            }

            for (size_t i = 0; i < value.length(); ++i) {
                const auto& entry = value[i];

                expectStructure(entry, "[ {'name': 'String', 'match': 'String'}, {'attribs': 'Array', 'pattern': 'String', 'flags': 'Array' } ]");
                checkTagName(entry["name"], result);

                auto name = entry["name"].stringValue();
                auto match = entry["match"].stringValue();

                if (match == "texture") {
                    expectMapEntry(entry, "pattern", EL::ValueType::String);
                    auto pattern = entry["pattern"].stringValue();
                    auto attribs = parseTagAttributes(entry["attribs"]);
                    auto matcher = std::make_unique<Model::TextureNameTagMatcher>(std::move(pattern));
                    result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
                } else if (match == "surfaceparm") {
                    parseSurfaceParmTag(name, entry, result);
                } else if (match == "contentflag") {
                    expectMapEntry(entry, "flags", EL::ValueType::Array);
                    const auto flagValue = parseFlagValue(entry["flags"], faceAttribsConfig.contentFlags);
                    auto attribs = parseTagAttributes(entry["attribs"]);
                    auto matcher = std::make_unique<Model::ContentFlagsTagMatcher>(flagValue);
                    result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
                } else if (match == "surfaceflag") {
                    expectMapEntry(entry, "flags", EL::ValueType::Array);
                    const auto flagValue = parseFlagValue(entry["flags"], faceAttribsConfig.surfaceFlags);
                    auto attribs = parseTagAttributes(entry["attribs"]);
                    auto matcher = std::make_unique<Model::SurfaceFlagsTagMatcher>(flagValue);
                    result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
                } else {
                    throw ParserException(entry.line(), entry.column(), "Unexpected smart tag match type '" + match + "'");
                }
            }
        }

        void GameConfigParser::parseSurfaceParmTag(const std::string& name, const EL::Value& value, std::vector<Model::SmartTag>& result) const {
            auto attribs = parseTagAttributes(value["attribs"]);
            std::unique_ptr<Model::SurfaceParmTagMatcher> matcher;
            if (m_version == 3) {
                expectMapEntry(value, "pattern", EL::ValueType::String);
                auto pattern = value["pattern"].stringValue();
                matcher = std::make_unique<Model::SurfaceParmTagMatcher>(std::move(pattern));
            } else {
                if (value["pattern"].type() == EL::ValueType::String) {
                    auto pattern = value["pattern"].stringValue();
                    matcher = std::make_unique<Model::SurfaceParmTagMatcher>(std::move(pattern));
                } else if (value["pattern"].type() == EL::ValueType::Array) {
                    auto patternVector = value["pattern"].asStringSet();
                    const kdl::vector_set<std::string> patternSet(patternVector.begin(), patternVector.end());
                    matcher = std::make_unique<Model::SurfaceParmTagMatcher>(patternSet);
                } else {
                    // Generate the type exception specifying Array as the
                    // expected type, since String is really a legacy type for
                    // backward compatibility.
                    expectMapEntry(value, "pattern", EL::ValueType::Array);
                }
            }
            result.emplace_back(std::move(name), std::move(attribs), std::move(matcher));
        }

        int GameConfigParser::parseFlagValue(const EL::Value& value, const Model::FlagsConfig& flags) const {
            const auto flagSet = value.asStringSet();
            int flagValue = 0;
            for (const std::string &currentName : flagSet) {
                const auto currentValue = flags.flagValue(currentName);
                flagValue |= currentValue;
            }
            return flagValue;
        }

        std::vector<Model::TagAttribute> GameConfigParser::parseTagAttributes(const EL::Value& value) const {
            auto result = std::vector<Model::TagAttribute>{};
            if (value.null()) {
                return result;
            }

            result.reserve(value.length());
            for (size_t i = 0; i < value.length(); ++i) {
                const auto& entry = value[i];
                const auto& name = entry.stringValue();

                if (name == Model::TagAttributes::Transparency.name()) {
                    result.push_back(Model::TagAttributes::Transparency);
                } else {
                    throw ParserException(entry.line(), entry.column(), "Unexpected tag attribute '" + name + "'");
                }
            }

            return result;
        }

        std::optional<vm::bbox3> GameConfigParser::parseSoftMapBounds(const EL::Value& value) const {
            if (value.null()) {
                return std::nullopt;
            }

            const auto bounds = parseSoftMapBoundsString(value.stringValue());
            if (!bounds.has_value()) {
                // If a bounds is provided in the config, it must be valid
                throw ParserException(value.line(), value.column(), "Can't parse soft map bounds '" + value.asString() + "'");
            }
            return bounds;
        }

        std::optional<vm::bbox3> parseSoftMapBoundsString(const std::string& string) {
            if (!vm::can_parse<double, 6u>(string)) {
                return std::nullopt;
            }

            const auto v = vm::parse<double, 6u>(string);
            const auto bounds = vm::bbox3(vm::vec3(v[0], v[1], v[2]),
                                          vm::vec3(v[3], v[4], v[5]));

            return { bounds };
        }

        std::string serializeSoftMapBoundsString(const vm::bbox3& bounds) {
            std::stringstream result;
            result << bounds.min << " " << bounds.max;
            return result.str();
        }
    }
}
