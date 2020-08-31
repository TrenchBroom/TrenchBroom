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

#ifndef TrenchBroom_GameConfig
#define TrenchBroom_GameConfig

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
#include <vector>

namespace TrenchBroom {
    namespace Model {
        struct MapFormatConfig {
            std::string format;
            IO::Path initialMap;

            MapFormatConfig(const std::string& i_format, const IO::Path& i_initialMap);
            MapFormatConfig();

            bool operator==(const MapFormatConfig& other) const;
        };

        struct PackageFormatConfig {
            std::vector<std::string> extensions;
            std::string format;

            PackageFormatConfig(const std::string& i_extension, const std::string& i_format);
            PackageFormatConfig(const std::vector<std::string>& i_extensions, const std::string& i_format);
            PackageFormatConfig();

            bool operator==(const PackageFormatConfig& other) const;
        };

        struct FileSystemConfig {
            IO::Path searchPath;
            PackageFormatConfig packageFormat;

            FileSystemConfig(const IO::Path& i_searchPath, const PackageFormatConfig& i_packageFormat);
            FileSystemConfig();

            bool operator==(const FileSystemConfig& other) const;
        };

        struct TexturePackageConfig {
            typedef enum {
                PT_File,
                PT_Directory,
                PT_Unset
            } PackageType;

            PackageType type;
            PackageFormatConfig fileFormat;
            IO::Path rootDirectory;

            explicit TexturePackageConfig(const PackageFormatConfig& i_format);
            explicit TexturePackageConfig(const IO::Path& directoryRoot);
            TexturePackageConfig();

            bool operator==(const TexturePackageConfig& other) const;
        };

        struct TextureConfig {
            TexturePackageConfig package;
            PackageFormatConfig format;
            IO::Path palette;
            std::string attribute;
            IO::Path shaderSearchPath;
            std::vector<std::string> excludes; // Glob patterns used to match texture names for exclusion

            TextureConfig(const TexturePackageConfig& i_package, const PackageFormatConfig& i_format, const IO::Path& i_palette, const std::string& i_attribute, const IO::Path& i_shaderSearchPath, const std::vector<std::string>& i_excludes);
            TextureConfig();

            bool operator==(const TextureConfig& other) const;
        };

        struct EntityConfig {
            std::vector<IO::Path> defFilePaths;
            std::vector<std::string> modelFormats;
            Color defaultColor;

            EntityConfig(const IO::Path& i_defFilePath, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor);
            EntityConfig(const std::vector<IO::Path>& i_defFilePaths, const std::vector<std::string>& i_modelFormats, const Color& i_defaultColor);
            EntityConfig();

            bool operator==(const EntityConfig& other) const;
        };

        struct FlagConfig {
            std::string name;
            std::string description;
            int value;

            FlagConfig(const std::string& i_name, const std::string& i_description, const int i_value);
            FlagConfig();

            bool operator==(const FlagConfig& other) const;
        };

        struct FlagsConfig {
            std::vector<FlagConfig> flags;

            FlagsConfig();
            explicit FlagsConfig(const std::vector<FlagConfig>& i_flags);

            int flagValue(const std::string& flagName) const;
            std::string flagName(size_t index) const;
            std::vector<std::string> flagNames(int mask = ~0) const;

            bool operator==(const FlagsConfig& other) const;
        };

        struct FaceAttribsConfig {
            FlagsConfig surfaceFlags;
            FlagsConfig contentFlags;
            BrushFaceAttributes defaults;

            FaceAttribsConfig();
            FaceAttribsConfig(const std::vector<FlagConfig>& i_surfaceFlags, const std::vector<FlagConfig>& i_contentFlags, const BrushFaceAttributes& i_defaults);
            FaceAttribsConfig(const FlagsConfig& i_surfaceFlags, const FlagsConfig& i_contentFlags, const BrushFaceAttributes& i_defaults);

            bool operator==(const FaceAttribsConfig& other) const;
        };

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
                std::optional<vm::bbox3> softMapBounds);

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

            CompilationConfig& compilationConfig();
            const CompilationConfig& compilationConfig() const;
            void setCompilationConfig(const CompilationConfig& compilationConfig);
            bool compilationConfigParseFailed() const;
            void setCompilationConfigParseFailed(bool failed) const;

            GameEngineConfig& gameEngineConfig();
            const GameEngineConfig& gameEngineConfig() const;
            void setGameEngineConfig(const GameEngineConfig& gameEngineConfig);
            bool gameEngineConfigParseFailed() const;
            void setGameEngineConfigParseFailed(bool failed) const;

            size_t maxPropertyLength() const;

            IO::Path findInitialMap(const std::string& formatName) const;
            IO::Path findConfigFile(const IO::Path& filePath) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameConfig) */
