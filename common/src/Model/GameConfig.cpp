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

#include "Ensure.h"
#include "IO/DiskFileSystem.h"

#include <kdl/string_utils.h>

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        MapFormatConfig::MapFormatConfig(const std::string& i_format, const IO::Path& i_initialMap) :
        format(i_format),
        initialMap(i_initialMap) {}

        MapFormatConfig::MapFormatConfig() = default;

        bool MapFormatConfig::operator==(const MapFormatConfig& other) const {
            return format == other.format && initialMap == other.initialMap;
        }

        PackageFormatConfig::PackageFormatConfig(const std::string& i_extension, const std::string& i_format) :
        extensions(1, i_extension),
        format(i_format) {}

        PackageFormatConfig::PackageFormatConfig(const std::vector<std::string>& i_extensions, const std::string& i_format) :
        extensions(i_extensions),
        format(i_format) {}

        PackageFormatConfig::PackageFormatConfig() = default;

        bool PackageFormatConfig::operator==(const PackageFormatConfig& other) const {
            return (extensions == other.extensions && format == other.format);
        }

        FileSystemConfig::FileSystemConfig(const IO::Path& i_searchPath, const PackageFormatConfig& i_packageFormat) :
        searchPath(i_searchPath),
        packageFormat(i_packageFormat) {}

        FileSystemConfig::FileSystemConfig() = default;

        bool FileSystemConfig::operator==(const FileSystemConfig& other) const {
            return (searchPath == other.searchPath &&
                    packageFormat == other.packageFormat);
        }

        TexturePackageConfig::TexturePackageConfig(const PackageFormatConfig& i_fileFormat) :
        type(PT_File),
        fileFormat(i_fileFormat) {}

        TexturePackageConfig::TexturePackageConfig(const IO::Path& i_rootDirectory) :
        type(PT_Directory),
        rootDirectory(i_rootDirectory) {}

        TexturePackageConfig::TexturePackageConfig() :
        type(PT_Unset) {}

        bool TexturePackageConfig::operator==(const TexturePackageConfig& other) const {
            return (type == other.type &&
                    fileFormat == other.fileFormat &&
                    rootDirectory == other.rootDirectory);
        }

        TextureConfig::TextureConfig(const TexturePackageConfig& i_package, const PackageFormatConfig& i_format, const IO::Path& i_palette, const std::string& i_attribute, const IO::Path& i_shaderSearchPath, const std::vector<std::string>& i_excludes) :
        package(i_package),
        format(i_format),
        palette(i_palette),
        attribute(i_attribute),
        shaderSearchPath(i_shaderSearchPath),
        excludes(i_excludes) {}

        TextureConfig::TextureConfig() = default;

        bool TextureConfig::operator==(const TextureConfig& other) const {
            return (package == other.package &&
                    format == other.format &&
                    palette == other.palette &&
                    attribute == other.attribute &&
                    shaderSearchPath == other.shaderSearchPath &&
                    excludes == other.excludes);
        }

        EntityConfig::EntityConfig(const IO::Path& i_defFilePath, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor) :
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {
            defFilePaths.push_back(i_defFilePath);
        }

        EntityConfig::EntityConfig(const std::vector<IO::Path>& i_defFilePaths, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor) :
        defFilePaths(i_defFilePaths),
        modelFormats(i_modelFormats),
        defaultColor(i_defaultColor) {}

        EntityConfig::EntityConfig() = default;

        bool EntityConfig::operator==(const EntityConfig& other) const {
            return (defFilePaths == other.defFilePaths &&
                    modelFormats == other.modelFormats &&
                    defaultColor == other.defaultColor);
        }

        FlagConfig::FlagConfig(const std::string& i_name, const std::string& i_description, const int i_value) :
        name(i_name),
        description(i_description),
        value(i_value) {}

        FlagConfig::FlagConfig() = default;

        bool FlagConfig::operator==(const FlagConfig& other) const {
            return (name == other.name &&
                    description == other.description &&
                    value == other.value);
        }

        FlagsConfig::FlagsConfig() = default;

        FlagsConfig::FlagsConfig(const std::vector<FlagConfig>& i_flags) :
        flags(i_flags) {}

        int FlagsConfig::flagValue(const std::string& flagName) const {
            for (size_t i = 0; i < flags.size(); ++i) {
                if (flags[i].name == flagName) {
                    return flags[i].value;
                }
            }
            return 0;
        }

        std::string FlagsConfig::flagName(const size_t index) const {
            ensure(index < flags.size(), "index out of range");
            return flags[index].name;
        }

        std::vector<std::string> FlagsConfig::flagNames(const int mask) const {
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

        bool FlagsConfig::operator==(const FlagsConfig& other) const {
            return flags == other.flags;
        }

        FaceAttribsConfig::FaceAttribsConfig() :
        defaults(BrushFaceAttributes::NoTextureName) {}

        FaceAttribsConfig::FaceAttribsConfig(const std::vector<FlagConfig>& i_surfaceFlags, const std::vector<FlagConfig>& i_contentFlags, const BrushFaceAttributes& i_defaults) :
        surfaceFlags(i_surfaceFlags),
        contentFlags(i_contentFlags),
        defaults(i_defaults) {}

        FaceAttribsConfig::FaceAttribsConfig(const FlagsConfig& i_surfaceFlags, const FlagsConfig& i_contentFlags, const BrushFaceAttributes& i_defaults) :
        surfaceFlags(i_surfaceFlags),
        contentFlags(i_contentFlags),
        defaults(i_defaults) {}

        bool FaceAttribsConfig::operator==(const FaceAttribsConfig& other) const {
            return (surfaceFlags == other.surfaceFlags &&
                    contentFlags == other.contentFlags &&
                    defaults == other.defaults);
        }

        GameConfig::GameConfig() :
        m_experimental(false),
        m_maxPropertyLength(1023),
        m_compilationConfigParseFailed(false),
        m_gameEngineConfigParseFailed(false) {}

        GameConfig::GameConfig(std::string name,
                               IO::Path path,
                               IO::Path icon,
                               const bool experimental,
                               std::vector<MapFormatConfig> fileFormats,
                               FileSystemConfig fileSystemConfig,
                               TextureConfig textureConfig,
                               EntityConfig entityConfig,
                               FaceAttribsConfig faceAttribsConfig,
                               std::vector<SmartTag> smartTags,
                               std::optional<vm::bbox3> softMapBounds,
                               std::vector<CompilationToolDescription> compilationToolDescriptions) :
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
        m_maxPropertyLength(1023),
        m_softMapBounds(std::move(softMapBounds)),
        m_compilationConfigParseFailed(false),
        m_gameEngineConfigParseFailed(false),
        m_compilationToolDescriptions(std::move(compilationToolDescriptions)) {
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

        const std::vector<MapFormatConfig>& GameConfig::fileFormats() const {
            return m_fileFormats;
        }

        const FileSystemConfig& GameConfig::fileSystemConfig() const {
            return m_fileSystemConfig;
        }

        const TextureConfig& GameConfig::textureConfig() const {
            return m_textureConfig;
        }

        const EntityConfig& GameConfig::entityConfig() const {
            return m_entityConfig;
        }

        const FaceAttribsConfig& GameConfig::faceAttribsConfig() const {
            return m_faceAttribsConfig;
        }

        const std::vector<SmartTag>& GameConfig::smartTags() const {
            return m_smartTags;
        }

        const std::optional<vm::bbox3>& GameConfig::softMapBounds() const {
            return m_softMapBounds;
        }

        const std::vector<CompilationToolDescription>& GameConfig::compilationToolDescriptions() const {
            return m_compilationToolDescriptions;
        }

        const CompilationConfig& GameConfig::compilationConfig() const {
            return m_compilationConfig;
        }

        void GameConfig::setCompilationConfig(const CompilationConfig& compilationConfig) {
            m_compilationConfig = compilationConfig;
        }

        bool GameConfig::compilationConfigParseFailed() const {
            return m_compilationConfigParseFailed;
        }

        void GameConfig::setCompilationConfigParseFailed(const bool failed) const {
            m_compilationConfigParseFailed = failed;
        }

        const GameEngineConfig& GameConfig::gameEngineConfig() const {
            return m_gameEngineConfig;
        }

        void GameConfig::setGameEngineConfig(const GameEngineConfig& gameEngineConfig) {
            m_gameEngineConfig = gameEngineConfig;
        }

        bool GameConfig::gameEngineConfigParseFailed() const {
            return m_gameEngineConfigParseFailed;
        }

        void GameConfig::setGameEngineConfigParseFailed(const bool failed) const {
            m_gameEngineConfigParseFailed = failed;
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
