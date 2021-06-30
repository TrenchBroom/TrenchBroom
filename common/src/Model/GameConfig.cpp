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
        bool operator==(const MapFormatConfig& lhs, const MapFormatConfig& rhs) {
            return lhs.format == rhs.format && lhs.initialMap == rhs.initialMap;
        }

        bool operator!=(const MapFormatConfig& lhs, const MapFormatConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs) {
            return lhs.extensions == rhs.extensions && lhs.format == rhs.format;
        }

        bool operator!=(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const FileSystemConfig& lhs, const FileSystemConfig& rhs) {
            return lhs.searchPath == rhs.searchPath && lhs.packageFormat == rhs.packageFormat;
        }

        bool operator!=(const FileSystemConfig& lhs, const FileSystemConfig& rhs) {
            return !(lhs == rhs);
        }

        TexturePackageConfig::TexturePackageConfig(PackageFormatConfig i_fileFormat) :
        type{PackageType::File},
        fileFormat{std::move(i_fileFormat)} {}

        TexturePackageConfig::TexturePackageConfig(IO::Path i_rootDirectory) :
        type{PackageType::Directory},
        rootDirectory{std::move(i_rootDirectory)} {}

        TexturePackageConfig::TexturePackageConfig() :
        type{PackageType::Unset} {}

        bool operator==(const TexturePackageConfig& lhs, const TexturePackageConfig& rhs) {
            return lhs.type == rhs.type &&
                   lhs.fileFormat == rhs.fileFormat &&
                   lhs.rootDirectory == rhs.rootDirectory;
        }

        bool operator!=(const TexturePackageConfig& lhs, const TexturePackageConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const TextureConfig& lhs, const TextureConfig& rhs) {
            return lhs.package == rhs.package &&
                   lhs.format == rhs.format &&
                   lhs.palette == rhs.palette &&
                   lhs.property == rhs.property &&
                   lhs.shaderSearchPath == rhs.shaderSearchPath &&
                   lhs.excludes == rhs.excludes;
        }

        bool operator!=(const TextureConfig& lhs, const TextureConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const EntityConfig& lhs, const EntityConfig& rhs) {
            return lhs.defFilePaths == rhs.defFilePaths &&
                   lhs.modelFormats == rhs.modelFormats &&
                   lhs.defaultColor == rhs.defaultColor;
        }

        bool operator!=(const EntityConfig& lhs, const EntityConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const FlagConfig& lhs, const FlagConfig& rhs) {
            return lhs.name == rhs.name &&
                   lhs.description == rhs.description &&
                   lhs.value == rhs.value;
        }

        bool operator!=(const FlagConfig& lhs, const FlagConfig& rhs) {
            return !(lhs == rhs);
        }

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

        bool operator==(const FlagsConfig& lhs, const FlagsConfig& rhs) {
            return lhs.flags == rhs.flags;
        }

        bool operator!=(const FlagsConfig& lhs, const FlagsConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs) {
            return lhs.surfaceFlags == rhs.surfaceFlags &&
                   lhs.contentFlags == rhs.contentFlags &&
                   lhs.defaults == rhs.defaults;
        }

        bool operator!=(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs) {
            return !(lhs == rhs);
        }

        bool operator==(const CompilationTool& lhs, const CompilationTool& rhs) {
            return lhs.name == rhs.name && 
                   lhs.description == rhs.description;
        }

        bool operator!=(const CompilationTool& lhs, const CompilationTool& rhs) {
            return !(lhs == rhs);
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
                               std::vector<CompilationTool> compilationTools) :
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
        m_compilationTools(std::move(compilationTools)) {
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

        const std::vector<CompilationTool>& GameConfig::compilationTools() const {
            return m_compilationTools;
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

        bool operator==(const GameConfig& lhs, const GameConfig& rhs) {
            return lhs.m_name == rhs.m_name &&
                   lhs.m_path == rhs.m_path &&
                   lhs.m_icon == rhs.m_icon &&
                   lhs.m_experimental == rhs.m_experimental &&
                   lhs.m_fileFormats == rhs.m_fileFormats &&
                   lhs.m_fileSystemConfig == rhs.m_fileSystemConfig &&
                   lhs.m_textureConfig == rhs.m_textureConfig &&
                   lhs.m_entityConfig == rhs.m_entityConfig &&
                   lhs.m_faceAttribsConfig == rhs.m_faceAttribsConfig &&
                   lhs.m_smartTags == rhs.m_smartTags &&
                   lhs.m_compilationConfig == rhs.m_compilationConfig &&
                   lhs.m_gameEngineConfig == rhs.m_gameEngineConfig &&
                   lhs.m_maxPropertyLength == rhs.m_maxPropertyLength &&
                   lhs.m_softMapBounds == rhs.m_softMapBounds &&
                   lhs.m_compilationConfigParseFailed == rhs.m_compilationConfigParseFailed &&
                   lhs.m_gameEngineConfigParseFailed == rhs.m_gameEngineConfigParseFailed &&
                   lhs.m_compilationTools == rhs.m_compilationTools;
        }

        bool operator!=(const GameConfig& lhs, const GameConfig& rhs) {
            return !(lhs == rhs);
        }
    }
}
