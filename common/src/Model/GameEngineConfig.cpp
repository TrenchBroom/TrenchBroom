/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "GameEngineConfig.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Model {
        GameEngineConfig::GameEngineConfig() {}
        
        GameEngineConfig::GameEngineConfig(const GameEngineProfile::List& profiles) :
        m_profiles(profiles) {}
        
        GameEngineConfig::GameEngineConfig(const GameEngineConfig& other) {
            m_profiles.reserve(other.m_profiles.size());
            
            GameEngineProfile::List::const_iterator it, end;
            for (it = other.m_profiles.begin(), end = other.m_profiles.end(); it != end; ++it) {
                const GameEngineProfile* original = *it;
                GameEngineProfile* clone = original->clone();
                m_profiles.push_back(clone);
            }
        }
        
        GameEngineConfig::~GameEngineConfig() {
            VectorUtils::clearAndDelete(m_profiles);
        }
        
        GameEngineConfig& GameEngineConfig::operator=(GameEngineConfig other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(GameEngineConfig& lhs, GameEngineConfig& rhs) {
            using std::swap;
            swap(lhs.m_profiles, rhs.m_profiles);
            swap(lhs.profilesDidChange, rhs.profilesDidChange);
        }
        
        size_t GameEngineConfig::profileCount() const {
            return m_profiles.size();
        }
        
        bool GameEngineConfig::hasProfile(const String& name) const {
            for (size_t i = 0; i < m_profiles.size(); ++i)
                if (m_profiles[i]->name() == name)
                    return true;
            return false;
        }

        GameEngineProfile* GameEngineConfig::profile(const size_t index) const {
            assert(index < profileCount());
            return m_profiles[index];
        }
        
        void GameEngineConfig::addProfile(GameEngineProfile* profile) {
            ensure(profile != NULL, "profile is null");
            m_profiles.push_back(profile);
            profilesDidChange();
        }
        
        void GameEngineConfig::removeProfile(const size_t index) {
            assert(index < profileCount());
            m_profiles[index]->profileWillBeRemoved();
            delete m_profiles[index];
            VectorUtils::erase(m_profiles, index);
            profilesDidChange();
        }
    }
}
