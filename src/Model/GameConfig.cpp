/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "GameConfig.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        GameConfig::FileSystemConfig::FileSystemConfig(const IO::Path& i_searchPath, const String& i_packageFormat) :
        searchPath(i_searchPath),
        packageFormat(i_packageFormat) {}
        
        GameConfig::TextureConfig::TextureConfig(const String& i_type, const String& i_property, const IO::Path& i_palette, const IO::Path& i_builtinTexturesSearchPath) :
        type(i_type),
        property(i_property),
        palette(i_palette),
        builtinTexturesSearchPath(i_builtinTexturesSearchPath) {}

        GameConfig::EntityConfig::EntityConfig(const IO::Path& i_defFilePath, const StringSet& i_modelFormats, const Color& i_defaultColor) :
        defFilePath(i_defFilePath),
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {}

        GameConfig::GameConfig() :
        m_fileSystemConfig(IO::Path(""), ""),
        m_textureConfig("", "", IO::Path(""), IO::Path("")),
        m_entityConfig(IO::Path(""), StringSet(), Color()) {}

        GameConfig::GameConfig(const String& name, const StringSet& fileFormats, const FileSystemConfig& fileSystemConfig, const TextureConfig& textureConfig, const EntityConfig& entityConfig) :
        m_name(name),
        m_fileFormats(fileFormats),
        m_fileSystemConfig(fileSystemConfig),
        m_textureConfig(textureConfig),
        m_entityConfig(entityConfig) {
            assert(!StringUtils::trim(m_name).empty());
        }
        
        const String& GameConfig::name() const {
            return m_name;
        }
        
        const StringSet& GameConfig::fileFormats() const {
            return m_fileFormats;
        }

        const GameConfig::FileSystemConfig& GameConfig::fileSystemConfig() const {
            return m_fileSystemConfig;
        }

        const GameConfig::TextureConfig& GameConfig::textureConfig() const {
            return m_textureConfig;
        }
        
        const GameConfig::EntityConfig& GameConfig::entityConfig() const {
            return m_entityConfig;
        }
    }
}
