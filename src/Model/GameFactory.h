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

#ifndef __TrenchBroom__GameFactory__
#define __TrenchBroom__GameFactory__

#include "StringUtils.h"
#include "Preference.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
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
            GamePtr createGame(const String& name) const;
            const GameConfig& gameConfig(const String& name) const;
            
            IO::Path gamePath(const String& gameName) const;
            void setGamePath(const String& gameName, const IO::Path& gamePath);

            GamePtr detectGame(const IO::Path& path) const;
        private:
            GameFactory();
            void loadGameConfigs();
            void loadGameConfig(const IO::FileSystem& fs, const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__GameFactory__) */
