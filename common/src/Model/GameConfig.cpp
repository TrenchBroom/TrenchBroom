/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
        GameConfig::PackageFormatConfig::PackageFormatConfig(const String& i_extension, const String& i_format) :
        extension(i_extension),
        format(i_format) {}

        GameConfig::PackageFormatConfig::PackageFormatConfig() {}

        bool GameConfig::PackageFormatConfig::operator==(const PackageFormatConfig& other) const {
            return (extension == other.extension &&
                    format == other.format);
        }

        GameConfig::FileSystemConfig::FileSystemConfig(const IO::Path& i_searchPath, const PackageFormatConfig& i_packageFormat) :
        searchPath(i_searchPath),
        packageFormat(i_packageFormat) {}
        
        GameConfig::FileSystemConfig::FileSystemConfig() {}
        
        bool GameConfig::FileSystemConfig::operator==(const FileSystemConfig& other) const {
            return (searchPath == other.searchPath &&
                    packageFormat == other.packageFormat);
        }

        GameConfig::TexturePackageConfig::TexturePackageConfig(const PackageFormatConfig& i_fileFormat) :
        type(PT_File),
        fileFormat(i_fileFormat) {}

        GameConfig::TexturePackageConfig::TexturePackageConfig(const IO::Path& i_rootDirectory) :
        type(PT_Directory),
        rootDirectory(i_rootDirectory) {}
        
        GameConfig::TexturePackageConfig::TexturePackageConfig() :
        type(PT_Unset) {}
        
        bool GameConfig::TexturePackageConfig::operator==(const TexturePackageConfig& other) const {
            return (type == other.type &&
                    fileFormat == other.fileFormat &&
                    rootDirectory == other.rootDirectory);
        }
        
        GameConfig::TextureConfig::TextureConfig(const TexturePackageConfig& i_package, const PackageFormatConfig& i_format, const IO::Path& i_palette, const String& i_attribute) :
        package(i_package),
        format(i_format),
        palette(i_palette),
        attribute(i_attribute) {}

        GameConfig::TextureConfig::TextureConfig() {}

        bool GameConfig::TextureConfig::operator==(const TextureConfig& other) const {
            return (package == other.package &&
                    format == other.format &&
                    palette == other.palette &&
                    attribute == other.attribute);
        }
        
        GameConfig::EntityConfig::EntityConfig(const IO::Path& i_defFilePath, const StringSet& i_modelFormats, const Color& i_defaultColor) :
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {
            defFilePaths.push_back(i_defFilePath);
        }

        GameConfig::EntityConfig::EntityConfig(const IO::Path::List& i_defFilePaths, const StringSet& i_modelFormats, const Color& i_defaultColor) :
        defFilePaths(i_defFilePaths),
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {}

        GameConfig::EntityConfig::EntityConfig() {}

        bool GameConfig::EntityConfig::operator==(const EntityConfig& other) const {
            return (defFilePaths == other.defFilePaths &&
                    modelFormats == other.modelFormats &&
                    defaultColor == other.defaultColor);
        }

        GameConfig::FlagConfig::FlagConfig(const String& i_name, const String& i_description) :
        name(i_name),
        description(i_description) {}

        GameConfig::FlagConfig::FlagConfig() {}

        bool GameConfig::FlagConfig::operator==(const FlagConfig& other) const {
            return (name == other.name &&
                    description == other.description);
        }
        
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
            ensure(index < flags.size(), "index out of range");
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
        
        bool GameConfig::FlagsConfig::operator==(const FlagsConfig& other) const {
            return flags == other.flags;
        }

        GameConfig::FaceAttribsConfig::FaceAttribsConfig() {}

        GameConfig::FaceAttribsConfig::FaceAttribsConfig(const FlagConfigList& i_surfaceFlags, const FlagConfigList& i_contentFlags) :
        surfaceFlags(i_surfaceFlags),
        contentFlags(i_contentFlags) {}

        bool GameConfig::FaceAttribsConfig::operator==(const FaceAttribsConfig& other) const {
            return (surfaceFlags == other.surfaceFlags &&
                    contentFlags == other.contentFlags);
        }

        GameConfig::GameConfig() :
        m_maxPropertyLength(1023) {}

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
        m_brushContentTypes(brushContentTypes),
        m_maxPropertyLength(1023) {
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

        CompilationConfig& GameConfig::compilationConfig() {
            return m_compilationConfig;
        }
        
        const CompilationConfig& GameConfig::compilationConfig() const {
            return m_compilationConfig;
        }

        void GameConfig::setCompilationConfig(const CompilationConfig& compilationConfig) {
            m_compilationConfig = compilationConfig;
        }

        GameEngineConfig& GameConfig::gameEngineConfig() {
            return m_gameEngineConfig;
        }
        
        const GameEngineConfig& GameConfig::gameEngineConfig() const {
            return m_gameEngineConfig;
        }
        
        void GameConfig::setGameEngineConfig(const GameEngineConfig& gameEngineConfig) {
            m_gameEngineConfig = gameEngineConfig;
        }

        size_t GameConfig::maxPropertyLength() const {
            return m_maxPropertyLength;
        }

        const IO::Path GameConfig::findConfigFile(const IO::Path& filePath) const {
            const IO::Path relPath = path().deleteLastComponent() + filePath;
//            if (IO::Disk::fileExists(relPath))
                return relPath;
//            return IO::SystemPaths::resourceDirectory() + filePath;
        }

        void GameConfig::addBrushContentType(const BrushContentType& contentType) {
            m_brushContentTypes.push_back(contentType);
        }
    }
}
