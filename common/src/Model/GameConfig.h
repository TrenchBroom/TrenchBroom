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
#include "StringSet.h"
#include "StringType.h"
#include "IO/Path.h"
#include "Model/CompilationConfig.h"
#include "Model/GameEngineConfig.h"
#include "Model/Model_Forward.h"
#include "Model/Tag.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class SmartTag;

        class GameConfig {
        public:
            struct MapFormatConfig {
                using List = std::vector<MapFormatConfig>;

                String format;
                IO::Path initialMap;

                MapFormatConfig(const String& i_format, const IO::Path& i_initialMap);
                MapFormatConfig();

                bool operator==(const MapFormatConfig& other) const;
            };

            struct PackageFormatConfig {
                using List = std::vector<PackageFormatConfig>;

                StringList extensions;
                String format;

                PackageFormatConfig(const String& i_extension, const String& i_format);
                PackageFormatConfig(const StringList& i_extensions, const String& i_format);
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
                String attribute;
                IO::Path shaderSearchPath;

                TextureConfig(const TexturePackageConfig& i_package, const PackageFormatConfig& i_format, const IO::Path& i_palette, const String& i_attribute, const IO::Path& i_shaderSearchPath);
                TextureConfig();

                bool operator==(const TextureConfig& other) const;
            };

            struct EntityConfig {
                IO::Path::List defFilePaths;
                StringSet modelFormats;
                Color defaultColor;

                EntityConfig(const IO::Path& i_defFilePath, const StringSet& i_modelFormats, const Color& i_defaultColor);
                EntityConfig(const IO::Path::List& i_defFilePaths, const StringSet& i_modelFormats, const Color& i_defaultColor);
                EntityConfig();

                bool operator==(const EntityConfig& other) const;
            };

            struct FlagConfig {
                String name;
                String description;

                FlagConfig(const String& i_name, const String& i_description);
                FlagConfig();

                bool operator==(const FlagConfig& other) const;
            };

            using FlagConfigList = std::vector<FlagConfig>;

            struct FlagsConfig {
                FlagConfigList flags;

                FlagsConfig();
                explicit FlagsConfig(const FlagConfigList& i_flags);

                int flagValue(const String& flagName) const;
                String flagName(size_t index) const;
                StringList flagNames(int mask = ~0) const;

                bool operator==(const FlagsConfig& other) const;
            };

            struct FaceAttribsConfig {
                FlagsConfig surfaceFlags;
                FlagsConfig contentFlags;

                FaceAttribsConfig();
                FaceAttribsConfig(const FlagConfigList& i_surfaceFlags, const FlagConfigList& i_contentFlags);

                bool operator==(const FaceAttribsConfig& other) const;
            };
        private:
            String m_name;
            IO::Path m_path;
            IO::Path m_icon;
            bool m_experimental;
            MapFormatConfig::List m_fileFormats;
            FileSystemConfig m_fileSystemConfig;
            TextureConfig m_textureConfig;
            EntityConfig m_entityConfig;
            FaceAttribsConfig m_faceAttribsConfig;
            std::vector<SmartTag> m_smartTags;
            CompilationConfig m_compilationConfig;
            GameEngineConfig m_gameEngineConfig;
            size_t m_maxPropertyLength;
        public:
            GameConfig();
            GameConfig(
                String name,
                IO::Path path,
                IO::Path icon,
                bool experimental,
                MapFormatConfig::List fileFormats,
                FileSystemConfig fileSystemConfig,
                TextureConfig textureConfig,
                EntityConfig entityConfig,
                FaceAttribsConfig faceAttribsConfig,
                std::vector<SmartTag> smartTags);

            const String& name() const;
            const IO::Path& path() const;
            const IO::Path& icon() const;
            bool experimental() const;
            const MapFormatConfig::List& fileFormats() const;
            const FileSystemConfig& fileSystemConfig() const;
            const TextureConfig& textureConfig() const;
            const EntityConfig& entityConfig() const;
            const FaceAttribsConfig& faceAttribsConfig() const;
            const std::vector<SmartTag>& smartTags() const;

            CompilationConfig& compilationConfig();
            const CompilationConfig& compilationConfig() const;
            void setCompilationConfig(const CompilationConfig& compilationConfig);

            GameEngineConfig& gameEngineConfig();
            const GameEngineConfig& gameEngineConfig() const;
            void setGameEngineConfig(const GameEngineConfig& gameEngineConfig);

            size_t maxPropertyLength() const;

            IO::Path findInitialMap(const String& formatName) const;
            IO::Path findConfigFile(const IO::Path& filePath) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameConfig) */
