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

#ifndef TrenchBroom_GameConfigParser
#define TrenchBroom_GameConfigParser

#include "Macros.h"
#include "EL/EL_Forward.h"
#include "IO/ConfigParserBase.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        struct EntityConfig;
        struct FaceAttribsConfig;
        struct FileSystemConfig;
        struct FlagConfig;
        struct FlagsConfig;
        class GameConfig;
        struct MapFormatConfig;
        struct PackageFormatConfig;
        class SmartTag;
        class TagAttribute;
        struct TextureConfig;
        struct TexturePackageConfig;
    }
    namespace IO {
        class Path;

        class GameConfigParser : public ConfigParserBase {
        public:
            GameConfigParser(const char* begin, const char* end, const Path& path);
            explicit GameConfigParser(const std::string& str, const Path& path = Path(""));

            Model::GameConfig parse();
        private:
            std::vector<Model::MapFormatConfig> parseMapFormatConfigs(const EL::Value& values) const;
            Model::FileSystemConfig parseFileSystemConfig(const EL::Value& values) const;
            Model::PackageFormatConfig parsePackageFormatConfig(const EL::Value& values) const;
            Model::TextureConfig parseTextureConfig(const EL::Value& values) const;
            Model::TexturePackageConfig parseTexturePackageConfig(const EL::Value& values) const;
            Model::EntityConfig parseEntityConfig(const EL::Value& values) const;
            Model::FaceAttribsConfig parseFaceAttribsConfig(const EL::Value& values) const;
            std::vector<Model::FlagConfig> parseFlagConfig(const EL::Value& values) const;
            Model::BrushFaceAttributes parseFaceAttribsDefaults(const EL::Value& value, const Model::FlagsConfig& surfaceFlags, const Model::FlagsConfig& contentFlags) const;
            std::vector<Model::SmartTag> parseTags(const EL::Value& value, const Model::FaceAttribsConfig& faceAttribsConfigs) const;

            void parseBrushTags(const EL::Value& value, std::vector<Model::SmartTag>& results) const;
            void parseFaceTags(const EL::Value& value, const Model::FaceAttribsConfig& faceAttribsConfig, std::vector<Model::SmartTag>& results) const;
            int parseFlagValue(const EL::Value& value, const Model::FlagsConfig& flags) const;
            std::vector<Model::TagAttribute> parseTagAttributes(const EL::Value& values) const;

            deleteCopyAndMove(GameConfigParser)
        };
    }
}

#endif /* defined(TrenchBroom_GameConfigParser) */
