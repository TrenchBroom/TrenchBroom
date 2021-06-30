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

#pragma once

#include "Color.h"
#include "FloatType.h"
#include "IO/Path.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/CompilationConfig.h"
#include "Model/GameEngineConfig.h"
#include "Model/Tag.h"

#include <vecmath/bbox.h>

#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        struct MapFormatConfig {
            std::string format;
            IO::Path initialMap;
        };

        bool operator==(const MapFormatConfig& lhs, const MapFormatConfig& rhs);
        bool operator!=(const MapFormatConfig& lhs, const MapFormatConfig& rhs);

        struct PackageFormatConfig {
            std::vector<std::string> extensions;
            std::string format;
        };

        bool operator==(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs);
        bool operator!=(const PackageFormatConfig& lhs, const PackageFormatConfig& rhs);

        struct FileSystemConfig {
            IO::Path searchPath;
            PackageFormatConfig packageFormat;
        };

        bool operator==(const FileSystemConfig& lhs, const FileSystemConfig& rhs);
        bool operator!=(const FileSystemConfig& lhs, const FileSystemConfig& rhs);

        struct TextureFilePackageConfig {
            PackageFormatConfig fileFormat;
        };

        bool operator==(const TextureFilePackageConfig& lhs, const TextureFilePackageConfig& rhs);
        bool operator!=(const TextureFilePackageConfig& lhs, const TextureFilePackageConfig& rhs);

        struct TextureDirectoryPackageConfig {
            IO::Path rootDirectory;
        };

        bool operator==(const TextureDirectoryPackageConfig& lhs, const TextureDirectoryPackageConfig& rhs);
        bool operator!=(const TextureDirectoryPackageConfig& lhs, const TextureDirectoryPackageConfig& rhs);

        using TexturePackageConfig = std::variant<TextureFilePackageConfig, TextureDirectoryPackageConfig>;

        IO::Path getRootDirectory(const TexturePackageConfig& texturePackageConfig);

        struct TextureConfig {
            TexturePackageConfig package;
            PackageFormatConfig format;
            IO::Path palette;
            std::string property;
            IO::Path shaderSearchPath;
            std::vector<std::string> excludes; // Glob patterns used to match texture names for exclusion
        };

        bool operator==(const TextureConfig& lhs, const TextureConfig& rhs);
        bool operator!=(const TextureConfig& lhs, const TextureConfig& rhs);

        struct EntityConfig {
            std::vector<IO::Path> defFilePaths;
            std::vector<std::string> modelFormats;
            Color defaultColor;
        };

        bool operator==(const EntityConfig& lhs, const EntityConfig& rhs);
        bool operator!=(const EntityConfig& lhs, const EntityConfig& rhs);

        struct FlagConfig {
            std::string name;
            std::string description;
            int value;
        };

        bool operator==(const FlagConfig& lhs, const FlagConfig& rhs);
        bool operator!=(const FlagConfig& lhs, const FlagConfig& rhs);

        struct FlagsConfig {
            std::vector<FlagConfig> flags;

            int flagValue(const std::string& flagName) const;
            std::string flagName(size_t index) const;
            std::vector<std::string> flagNames(int mask = ~0) const;
        };

        bool operator==(const FlagsConfig& lhs, const FlagsConfig& rhs);
        bool operator!=(const FlagsConfig& lhs, const FlagsConfig& rhs);

        struct FaceAttribsConfig {
            FlagsConfig surfaceFlags;
            FlagsConfig contentFlags;
            BrushFaceAttributes defaults{BrushFaceAttributes::NoTextureName};
        };

        bool operator==(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs);
        bool operator!=(const FaceAttribsConfig& lhs, const FaceAttribsConfig& rhs);

        struct CompilationTool {
            std::string name;
            std::optional<std::string> description;
        };

        bool operator==(const CompilationTool& lhs, const CompilationTool& rhs);
        bool operator!=(const CompilationTool& lhs, const CompilationTool& rhs);

        class GameConfig {
        private:
            std::string m_name;
            IO::Path m_path;
            IO::Path m_icon;
            bool m_experimental;
            std::vector<MapFormatConfig> m_fileFormats;
            FileSystemConfig m_fileSystemConfig;
            TextureConfig m_textureConfig;
            EntityConfig m_entityConfig;
            FaceAttribsConfig m_faceAttribsConfig;
            std::vector<SmartTag> m_smartTags;
            CompilationConfig m_compilationConfig;
            GameEngineConfig m_gameEngineConfig;
            size_t m_maxPropertyLength;
            std::optional<vm::bbox3> m_softMapBounds;
            mutable bool m_compilationConfigParseFailed;
            mutable bool m_gameEngineConfigParseFailed;
            std::vector<CompilationTool> m_compilationTools;
        public:
            GameConfig();
            GameConfig(
                std::string name,
                IO::Path path,
                IO::Path icon,
                bool experimental,
                std::vector<MapFormatConfig> fileFormats,
                FileSystemConfig fileSystemConfig,
                TextureConfig textureConfig,
                EntityConfig entityConfig,
                FaceAttribsConfig faceAttribsConfig,
                std::vector<SmartTag> smartTags,
                std::optional<vm::bbox3> softMapBounds,
                std::vector<CompilationTool> compilationTools);

            const std::string& name() const;
            const IO::Path& path() const;
            const IO::Path& icon() const;
            bool experimental() const;
            const std::vector<MapFormatConfig>& fileFormats() const;
            const FileSystemConfig& fileSystemConfig() const;
            const TextureConfig& textureConfig() const;
            const EntityConfig& entityConfig() const;
            const FaceAttribsConfig& faceAttribsConfig() const;
            const std::vector<SmartTag>& smartTags() const;
            const std::optional<vm::bbox3>& softMapBounds() const;
            const std::vector<CompilationTool>& compilationTools() const;

            const CompilationConfig& compilationConfig() const;
            void setCompilationConfig(const CompilationConfig& compilationConfig);
            bool compilationConfigParseFailed() const;
            void setCompilationConfigParseFailed(bool failed) const;

            const GameEngineConfig& gameEngineConfig() const;
            void setGameEngineConfig(const GameEngineConfig& gameEngineConfig);
            bool gameEngineConfigParseFailed() const;
            void setGameEngineConfigParseFailed(bool failed) const;

            size_t maxPropertyLength() const;

            IO::Path findInitialMap(const std::string& formatName) const;
            IO::Path findConfigFile(const IO::Path& filePath) const;

            friend bool operator==(const GameConfig& lhs, const GameConfig& rhs);
            friend bool operator!=(const GameConfig& lhs, const GameConfig& rhs);
        };
    }
}

