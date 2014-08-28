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

#include "GameConfig.h"

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/SystemPaths.h"

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
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {
            defFilePaths.push_back(i_defFilePath);
        }

        GameConfig::EntityConfig::EntityConfig(const IO::Path::List& i_defFilePaths, const StringSet& i_modelFormats, const Color& i_defaultColor) :
        defFilePaths(i_defFilePaths),
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {}

        GameConfig::FlagConfig::FlagConfig(const String& i_name, const String& i_description) :
        name(i_name),
        description(i_description) {}

        GameConfig::FlagsConfig::FlagsConfig() {}

        GameConfig::FlagsConfig::FlagsConfig(const FlagConfigList& i_flags) :
        flags(i_flags) {}

        int GameConfig::FlagsConfig::flagValue(const String& flagName) const {
            for (size_t i = 0; i < flags.size(); ++i) {
                if (flags[i].name == flagName)
                    return static_cast<int>(1 << i);
            }
            return 0;
        }

        String GameConfig::FlagsConfig::flagName(const size_t index) const {
            assert(index < flags.size());
            return flags[index].name;
        }
        
        StringList GameConfig::FlagsConfig::flagNames(const int mask) const {
            if (mask == 0)
                return EmptyStringList;
            
            StringList names;
            for (size_t i = 0; i < flags.size(); ++i) {
                if (mask & (1 << i))
                    names.push_back(flags[i].name);
            }
            return names;
        }
        
        GameConfig::FaceAttribsConfig::FaceAttribsConfig() {}

        GameConfig::FaceAttribsConfig::FaceAttribsConfig(const FlagConfigList& i_surfaceFlags, const FlagConfigList& i_contentFlags) :
        surfaceFlags(i_surfaceFlags),
        contentFlags(i_contentFlags) {}

        GameConfig::GameConfig() :
        m_path(IO::Path("")),
        m_icon(IO::Path("")),
        m_fileSystemConfig(IO::Path(""), ""),
        m_textureConfig("", "", IO::Path(""), IO::Path("")),
        m_entityConfig(IO::Path(""), StringSet(), Color()) {}

        GameConfig::GameConfig(const String& name,
                               const IO::Path& path,
                               const IO::Path& icon,
                               const StringList& fileFormats,
                               const FileSystemConfig& fileSystemConfig,
                               const TextureConfig& textureConfig,
                               const EntityConfig& entityConfig,
                               const FaceAttribsConfig& faceAttribsConfig,
                               const BrushContentType::List& brushContentTypes) :
        m_name(name),
        m_path(path),
        m_icon(icon),
        m_fileFormats(fileFormats),
        m_fileSystemConfig(fileSystemConfig),
        m_textureConfig(textureConfig),
        m_entityConfig(entityConfig),
        m_faceAttribsConfig(faceAttribsConfig),
        m_brushContentTypes(brushContentTypes) {
            assert(!StringUtils::trim(m_name).empty());
            assert(m_path.isEmpty() || m_path.isAbsolute());
        }
        
        const String& GameConfig::name() const {
            return m_name;
        }
        
        const IO::Path& GameConfig::path() const {
            return m_path;
        }
    
        const IO::Path& GameConfig::icon() const {
            return m_icon;
        }

        const StringList& GameConfig::fileFormats() const {
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

        const GameConfig::FaceAttribsConfig& GameConfig::faceAttribsConfig() const {
            return m_faceAttribsConfig;
        }

        const BrushContentType::List& GameConfig::brushContentTypes() const {
            return m_brushContentTypes;
        }

        const IO::Path GameConfig::findConfigFile(const IO::Path& filePath) const {
            const IO::Path relPath = path().deleteLastComponent() + filePath;
//            if (IO::Disk::fileExists(relPath))
                return relPath;
//            return IO::SystemPaths::resourceDirectory() + filePath;
        }
    }
}
