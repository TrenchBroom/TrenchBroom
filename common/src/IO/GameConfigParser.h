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
#include "StringType.h"
#include "IO/ConfigParserBase.h"
#include "Model/GameConfig.h"

#include <iostream>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class SmartTag;
        class TagAttribute;
    }

    namespace IO {
        class ParserStatus;

        class GameConfigParser : public ConfigParserBase {
        public:
            GameConfigParser(const char* begin, const char* end, const Path& path);
            explicit GameConfigParser(const String& str, const Path& path = Path(""));

            Model::GameConfig parse();
        private:
            Model::GameConfig::MapFormatConfig::List parseMapFormatConfigs(const EL::Value& values) const;
            Model::GameConfig::FileSystemConfig parseFileSystemConfig(const EL::Value& values) const;
            Model::GameConfig::PackageFormatConfig parsePackageFormatConfig(const EL::Value& values) const;
            Model::GameConfig::TextureConfig parseTextureConfig(const EL::Value& values) const;
            Model::GameConfig::TexturePackageConfig parseTexturePackageConfig(const EL::Value& values) const;
            Model::GameConfig::EntityConfig parseEntityConfig(const EL::Value& values) const;
            Model::GameConfig::FaceAttribsConfig parseFaceAttribsConfig(const EL::Value& values) const;
            Model::GameConfig::FlagConfigList parseFlagConfig(const EL::Value& values) const;
            std::vector<Model::SmartTag> parseTags(const EL::Value& value, const Model::GameConfig::FaceAttribsConfig& faceAttribsConfigs) const;

            void parseBrushTags(const EL::Value& value, std::vector<Model::SmartTag>& results) const;
            void parseFaceTags(const EL::Value& value, const Model::GameConfig::FaceAttribsConfig& faceAttribsConfig, std::vector<Model::SmartTag>& results) const;
            int parseFlagValue(const EL::Value& value, const Model::GameConfig::FlagsConfig& flags) const;
            std::vector<Model::TagAttribute> parseTagAttributes(const EL::Value& values) const;

            deleteCopyAndMove(GameConfigParser)
        };
    }
}

#endif /* defined(TrenchBroom_GameConfigParser) */
