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

#ifndef TrenchBroom_GameFactory
#define TrenchBroom_GameFactory

#include "StringUtils.h"
#include "Preference.h"
#include "IO/Path.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class WritableDiskFileSystem;
    }
    
    namespace Model {
        class GameFactory {
        private:
            using ConfigMap = std::map<String, GameConfig>;
            using GamePathMap = std::map<String, Preference<IO::Path> >;

            std::unique_ptr<IO::WritableDiskFileSystem> m_configFS;

            StringList m_names;
            ConfigMap m_configs;
            mutable GamePathMap m_gamePaths;
            mutable GamePathMap m_defaultEngines;
        public:
            ~GameFactory();
            
            static GameFactory& instance();
            
            const StringList& gameList() const;
            size_t gameCount() const;
            GameSPtr createGame(const String& gameName, Logger& logger);
            
            StringList fileFormats(const String& gameName) const;
            IO::Path iconPath(const String& gameName) const;
            IO::Path gamePath(const String& gameName) const;
            bool setGamePath(const String& gameName, const IO::Path& gamePath);
            bool isGamePathPreference(const String& gameName, const IO::Path& prefPath) const;

            GameConfig& gameConfig(const String& gameName);
            const GameConfig& gameConfig(const String& gameName) const;
            
            std::pair<String, MapFormat> detectGame(const IO::Path& path) const;
        private:
            GameFactory();
            void initializeFileSystem();
            void loadGameConfigs();
            void loadGameConfig(const IO::Path& path);
            void loadCompilationConfig(GameConfig& gameConfig);
            void loadGameEngineConfig(GameConfig& gameConfig);
            
            void writeCompilationConfigs();
            void writeCompilationConfig(const GameConfig& gameConfig);

            void writeGameEngineConfigs();
            void writeGameEngineConfig(const GameConfig& gameConfig);


        };
    }
}

#endif /* defined(TrenchBroom_GameFactory) */
