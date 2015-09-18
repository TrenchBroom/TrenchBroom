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

#ifndef TrenchBroom_GameConfig
#define TrenchBroom_GameConfig

#include "Color.h"
#include "StringUtils.h"
#include "IO/Path.h"
#include "Model/BrushContentType.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class GameConfig {
        public:
            struct FileSystemConfig {
                IO::Path searchPath;
                String packageFormat;
                
                FileSystemConfig(const IO::Path& i_searchPath, const String& i_packageFormat);
            };
            
            struct TextureConfig {
                String type;
                String attribute;
                IO::Path palette;
                IO::Path builtinTexturesSearchPath;
                
                TextureConfig(const String& i_type, const String& i_attribute, const IO::Path& i_palette, const IO::Path& i_builtinTexturesSearchPath);
            };
            
            struct EntityConfig {
                IO::Path::List defFilePaths;
                StringSet modelFormats;
                Color defaultColor;
                
                EntityConfig(const IO::Path& i_defFilePath, const StringSet& i_modelFormats, const Color& i_defaultColor);
                EntityConfig(const IO::Path::List& i_defFilePaths, const StringSet& i_modelFormats, const Color& i_defaultColor);
            };
            
            struct FlagConfig {
                String name;
                String description;
                
                FlagConfig(const String& i_name, const String& i_description);
            };
            
            typedef std::vector<FlagConfig> FlagConfigList;
            
            struct FlagsConfig {
                FlagConfigList flags;
                
                FlagsConfig();
                FlagsConfig(const FlagConfigList& i_flags);

                int flagValue(const String& flagName) const;
                String flagName(size_t index) const;
                StringList flagNames(int mask = ~0) const;
            };
            
            struct FaceAttribsConfig {
                FlagsConfig surfaceFlags;
                FlagsConfig contentFlags;
                
                FaceAttribsConfig();
                FaceAttribsConfig(const FlagConfigList& i_surfaceFlags, const FlagConfigList& i_contentFlags);
                
            };
        private:
            String m_name;
            IO::Path m_path;
            IO::Path m_icon;
            StringList m_fileFormats;
            FileSystemConfig m_fileSystemConfig;
            TextureConfig m_textureConfig;
            EntityConfig m_entityConfig;
            FaceAttribsConfig m_faceAttribsConfig;
            BrushContentType::List m_brushContentTypes;
        public:
            GameConfig();
            GameConfig(const String& name, const IO::Path& path, const IO::Path& icon, const StringList& fileFormats, const FileSystemConfig& fileSystemConfig, const TextureConfig& textureConfig, const EntityConfig& entityConfig, const FaceAttribsConfig& faceAttribsConfig, const BrushContentType::List& brushContentTypes);
            
            const String& name() const;
            const IO::Path& path() const;
            const IO::Path& icon() const;
            const StringList& fileFormats() const;
            const FileSystemConfig& fileSystemConfig() const;
            const TextureConfig& textureConfig() const;
            const EntityConfig& entityConfig() const;
            const FaceAttribsConfig& faceAttribsConfig() const;
            const BrushContentType::List& brushContentTypes() const;
            const IO::Path findConfigFile(const IO::Path& filePath) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameConfig) */
