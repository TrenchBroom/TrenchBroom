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
#include "StringUtils.h"
#include "IO/ConfigParserBase.h"
#include "Model/BrushContentType.h"
#include "Model/GameConfig.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class GameConfigParser : public ConfigParserBase {
        public:
            GameConfigParser(const char* begin, const char* end, const Path& path);
            GameConfigParser(const String& str, const Path& path = Path(""));
            
            Model::GameConfig parse();
        private:
            Model::GameConfig::MapFormatConfig::List parseMapFormatConfigs(const EL::Value& value) const;
            Model::GameConfig::FileSystemConfig parseFileSystemConfig(const EL::Value& value) const;
            Model::GameConfig::PackageFormatConfig parsePackageFormatConfig(const EL::Value& value) const;
            Model::GameConfig::TextureConfig parseTextureConfig(const EL::Value& value) const;
            Model::GameConfig::TexturePackageConfig parseTexturePackageConfig(const EL::Value& value) const;
            Model::GameConfig::EntityConfig parseEntityConfig(const EL::Value& value) const;
            Model::GameConfig::FaceAttribsConfig parseFaceAttribsConfig(const EL::Value& value) const;
            Model::GameConfig::FlagConfigList parseFlagConfig(const EL::Value& value) const;
            Model::BrushContentType::List parseBrushContentTypes(const EL::Value& value, const Model::GameConfig::FaceAttribsConfig& faceAttribsConfig) const;
            
            deleteCopyAndMove(GameConfigParser)
        };
    }
}

#endif /* defined(TrenchBroom_GameConfigParser) */
