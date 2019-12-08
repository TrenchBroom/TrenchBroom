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

#include "GameConfig.h"

#include "IO/DiskFileSystem.h"

#include <kdl/string_format.h>

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GameConfig::MapFormatConfig::MapFormatConfig(const std::string& i_format, const IO::Path& i_initialMap) :
        format(i_format),
        initialMap(i_initialMap) {}

        GameConfig::MapFormatConfig::MapFormatConfig() = default;

        bool GameConfig::MapFormatConfig::operator==(const MapFormatConfig& other) const {
            return format == other.format && initialMap == other.initialMap;
        }

        GameConfig::PackageFormatConfig::PackageFormatConfig(const std::string& i_extension, const std::string& i_format) :
        extensions(1, i_extension),
        format(i_format) {}

        GameConfig::PackageFormatConfig::PackageFormatConfig(const std::vector<std::string>& i_extensions, const std::string& i_format) :
        extensions(i_extensions),
        format(i_format) {}

        GameConfig::PackageFormatConfig::PackageFormatConfig() = default;

        bool GameConfig::PackageFormatConfig::operator==(const PackageFormatConfig& other) const {
            return (extensions == other.extensions && format == other.format);
        }

        GameConfig::FileSystemConfig::FileSystemConfig(const IO::Path& i_searchPath, const PackageFormatConfig& i_packageFormat) :
        searchPath(i_searchPath),
        packageFormat(i_packageFormat) {}

        GameConfig::FileSystemConfig::FileSystemConfig() = default;

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

        GameConfig::TextureConfig::TextureConfig(const TexturePackageConfig& i_package, const PackageFormatConfig& i_format, const IO::Path& i_palette, const std::string& i_attribute, const IO::Path& i_shaderSearchPath) :
        package(i_package),
        format(i_format),
        palette(i_palette),
        attribute(i_attribute),
        shaderSearchPath(i_shaderSearchPath) {}

        GameConfig::TextureConfig::TextureConfig() = default;

        bool GameConfig::TextureConfig::operator==(const TextureConfig& other) const {
            return (package == other.package &&
                    format == other.format &&
                    palette == other.palette &&
                    attribute == other.attribute &&
                    shaderSearchPath == other.shaderSearchPath);
        }

        GameConfig::EntityConfig::EntityConfig(const IO::Path& i_defFilePath, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor) :
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {
            defFilePaths.push_back(i_defFilePath);
        }

        GameConfig::EntityConfig::EntityConfig(const std::vector<IO::Path>& i_defFilePaths, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor) :
        defFilePaths(i_defFilePaths),
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {}

        GameConfig::EntityConfig::EntityConfig() = default;

        bool GameConfig::EntityConfig::operator==(const EntityConfig& other) const {
            return (defFilePaths == other.defFilePaths &&
                    modelFormats == other.modelFormats &&
                    defaultColor == other.defaultColor);
        }

        GameConfig::FlagConfig::FlagConfig(const std::string& i_name, const std::string& i_description) :
        name(i_name),
        description(i_description) {}

        GameConfig::FlagConfig::FlagConfig() = default;

        bool GameConfig::FlagConfig::operator==(const FlagConfig& other) const {
            return (name == other.name &&
                    description == other.description);
        }

        GameConfig::FlagsConfig::FlagsConfig() = default;

        GameConfig::FlagsConfig::FlagsConfig(const FlagConfigList& i_flags) :
        flags(i_flags) {}

        int GameConfig::FlagsConfig::flagValue(const std::string& flagName) const {
            for (size_t i = 0; i < flags.size(); ++i) {
                if (flags[i].name == flagName) {
                    return static_cast<int>(1 << i);
                }
            }
            return 0;
        }

        std::string GameConfig::FlagsConfig::flagName(const size_t index) const {
            ensure(index < flags.size(), "index out of range");
            return flags[index].name;
        }

        std::vector<std::string> GameConfig::FlagsConfig::flagNames(const int mask) const {
            if (mask == 0) {
                return {};
            }

            std::vector<std::string> names;
            for (size_t i = 0; i < flags.size(); ++i) {
                if (mask & (1 << i)) {
                    names.push_back(flags[i].name);
                }
            }
            return names;
        }

        bool GameConfig::FlagsConfig::operator==(const FlagsConfig& other) const {
            return flags == other.flags;
        }

        GameConfig::FaceAttribsConfig::FaceAttribsConfig() = default;

        GameConfig::FaceAttribsConfig::FaceAttribsConfig(const FlagConfigList& i_surfaceFlags, const FlagConfigList& i_contentFlags) :
        surfaceFlags(i_surfaceFlags),
        contentFlags(i_contentFlags) {}

        bool GameConfig::FaceAttribsConfig::operator==(const FaceAttribsConfig& other) const {
            return (surfaceFlags == other.surfaceFlags &&
                    contentFlags == other.contentFlags);
        }

        GameConfig::GameConfig() :
        m_experimental(false),
        m_maxPropertyLength(1023) {}

        GameConfig::GameConfig(std::string name,
                               IO::Path path,
                               IO::Path icon,
                               const bool experimental,
                               MapFormatConfig::List fileFormats,
                               FileSystemConfig fileSystemConfig,
                               TextureConfig textureConfig,
                               EntityConfig entityConfig,
                               FaceAttribsConfig faceAttribsConfig,
                               std::vector<SmartTag> smartTags) :
        m_name(std::move(name)),
        m_path(std::move(path)),
        m_icon(std::move(icon)),
        m_experimental(experimental),
        m_fileFormats(std::move(fileFormats)),
        m_fileSystemConfig(std::move(fileSystemConfig)),
        m_textureConfig(std::move(textureConfig)),
        m_entityConfig(std::move(entityConfig)),
        m_faceAttribsConfig(std::move(faceAttribsConfig)),
        m_smartTags(std::move(smartTags)),
        m_maxPropertyLength(1023) {
            assert(!kdl::str_trim(m_name).empty());
            assert(m_path.isEmpty() || m_path.isAbsolute());
        }

        const std::string& GameConfig::name() const {
            return m_name;
        }

        const IO::Path& GameConfig::path() const {
            return m_path;
        }

        const IO::Path& GameConfig::icon() const {
            return m_icon;
        }

        bool GameConfig::experimental() const {
            return m_experimental;
        }

        const GameConfig::MapFormatConfig::List& GameConfig::fileFormats() const {
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

        const std::vector<SmartTag>& GameConfig::smartTags() const {
            return m_smartTags;
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

        IO::Path GameConfig::findInitialMap(const std::string& formatName) const {
            for (const auto& format : m_fileFormats) {
                if (format.format == formatName) {
                    if (!format.initialMap.isEmpty()) {
                        return findConfigFile(format.initialMap);
                    } else {
                        break;
                    }
                }
            }
            return IO::Path("");
        }

        IO::Path GameConfig::findConfigFile(const IO::Path& filePath) const {
            return path().deleteLastComponent() + filePath;
        }
    }
}
