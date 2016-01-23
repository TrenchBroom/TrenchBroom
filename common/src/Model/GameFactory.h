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

#ifndef TrenchBroom_GameFactory
#define TrenchBroom_GameFactory

#include "StringUtils.h"
#include "Preference.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class DiskFileSystem;
    }
    
    namespace Model {
        class GameFactory {
        private:
            typedef std::map<String, GameConfig> ConfigMap;
            typedef std::map<String, Preference<IO::Path> > GamePathMap;
            
            StringList m_names;
            ConfigMap m_configs;
            mutable GamePathMap m_gamePaths;
        public:
            static GameFactory& instance();
            
            const StringList& gameList() const;
            size_t gameCount() const;
            GamePtr createGame(const String& gameName) const;
            
            const StringList& fileFormats(const String& gameName) const;
            IO::Path iconPath(const String& gameName) const;
            IO::Path gamePath(const String& gameName) const;
            void setGamePath(const String& gameName, const IO::Path& gamePath);
            bool isGamePathPreference(const String& gameName, const IO::Path& prefPath) const;

            std::pair<String, MapFormat::Type> detectGame(const IO::Path& path) const;
        private:
            GameFactory();
            void loadGameConfigs();
            void loadGameConfig(const IO::DiskFileSystem& fs, const IO::Path& path);
            const GameConfig& gameConfig(const String& name) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameFactory) */
