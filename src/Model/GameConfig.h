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

#ifndef __TrenchBroom__GameConfig__
#define __TrenchBroom__GameConfig__

#include "Color.h"
#include "StringUtils.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"

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
                String property;
                IO::Path palette;
                IO::Path builtinTexturesSearchPath;
                
                TextureConfig(const String& i_type, const String& i_property, const IO::Path& i_palette, const IO::Path& i_builtinTexturesSearchPath);
            };
            
            struct EntityConfig {
                IO::Path defFilePath;
                StringSet modelFormats;
                Color defaultColor;
                
                EntityConfig(const IO::Path& i_defFilePath, const StringSet& i_modelFormats, const Color& i_defaultColor);
            };
        private:
            String m_name;
            IO::Path m_icon;
            StringSet m_fileFormats;
            FileSystemConfig m_fileSystemConfig;
            TextureConfig m_textureConfig;
            EntityConfig m_entityConfig;
        public:
            GameConfig();
            GameConfig(const String& name, const IO::Path& icon, const StringSet& fileFormats, const FileSystemConfig& fileSystemConfig, const TextureConfig& textureConfig, const EntityConfig& entityConfig);
            
            const String& name() const;
            const IO::Path& icon() const;
            const StringSet& fileFormats() const;
            const FileSystemConfig& fileSystemConfig() const;
            const TextureConfig& textureConfig() const;
            const EntityConfig& entityConfig() const;
        };
    }
}

#endif /* defined(__TrenchBroom__GameConfig__) */
