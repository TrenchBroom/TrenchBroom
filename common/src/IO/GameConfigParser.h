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

#ifndef TrenchBroom_GameConfigParser
#define TrenchBroom_GameConfigParser

#include "StringUtils.h"
#include "IO/ConfigParser.h"
#include "IO/Path.h"
#include "Model/BrushContentType.h"
#include "Model/GameConfig.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class GameConfigParser {
        private:
            ConfigParser m_parser;
            Path m_path;
        public:
            GameConfigParser(const char* begin, const char* end, const Path& path);
            GameConfigParser(const String& str, const Path& path = Path(""));
            
            Model::GameConfig parse();
        private:
            Model::GameConfig::FileSystemConfig parseFileSystemConfig(const ConfigTable& table) const;
            Model::GameConfig::TextureConfig parseTextureConfig(const ConfigTable& table) const;
            Model::GameConfig::EntityConfig parseEntityConfig(const ConfigTable& table) const;
            Model::GameConfig::FaceAttribsConfig parseFaceAttribsConfig(const ConfigTable& table) const;
            Model::GameConfig::FlagConfigList parseFlagConfig(const ConfigList& list) const;
            Model::BrushContentType::List parseBrushContentTypes(const ConfigList& list, const Model::GameConfig::FaceAttribsConfig& faceAttribsConfig) const;
            StringSet parseSet(const ConfigList& list) const;
            StringList parseList(const ConfigList& list) const;
            
            void expectEntry(int typeMask, const ConfigEntry& entry) const;
            void expectTableEntry(const String& key, int typeMask, const ConfigTable& table) const;
            String typeNames(int typeMask) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameConfigParser) */
