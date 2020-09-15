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

#ifndef GameEngineConfig_h
#define GameEngineConfig_h

#include "Notifier.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;

        class GameEngineConfig {
        private:
            std::vector<std::unique_ptr<GameEngineProfile>> m_profiles;
        public:
            mutable Notifier<> profilesDidChange;
        public:
            GameEngineConfig();
            GameEngineConfig(std::vector<std::unique_ptr<GameEngineProfile>> profiles);
            GameEngineConfig(const GameEngineConfig& other);
            ~GameEngineConfig();

            GameEngineConfig& operator=(GameEngineConfig other);
            friend void swap(GameEngineConfig& lhs, GameEngineConfig& rhs);

            size_t profileCount() const;
            bool hasProfile(const std::string& name) const;
            GameEngineProfile* profile(size_t index) const;

            void addProfile(std::unique_ptr<GameEngineProfile> profile);
            void removeProfile(size_t index);
        };
    }
}

#endif /* GameEngineConfig_h */
