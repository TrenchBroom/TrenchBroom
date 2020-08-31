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

#include "GameEngineConfig.h"

#include "Ensure.h"
#include "Model/GameEngineProfile.h"

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GameEngineConfig::GameEngineConfig() {}

        GameEngineConfig::GameEngineConfig(std::vector<std::unique_ptr<GameEngineProfile>> profiles) :
        m_profiles(std::move(profiles)) {}

        GameEngineConfig::GameEngineConfig(const GameEngineConfig& other) {
            m_profiles.reserve(other.m_profiles.size());

            for (const auto& original : other.m_profiles) {
                auto clone = original->clone();
                ensure(clone->parent() == nullptr, "profile already has a parent");

                clone->setParent(this);
                m_profiles.push_back(std::move(clone));
            }
        }

        GameEngineConfig::~GameEngineConfig() = default;

        GameEngineConfig& GameEngineConfig::operator=(GameEngineConfig other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(GameEngineConfig& lhs, GameEngineConfig& rhs) {
            using std::swap;
            swap(lhs.m_profiles, rhs.m_profiles);
            swap(lhs.profilesDidChange, rhs.profilesDidChange);
            swap(lhs.configDidChange, rhs.configDidChange);
        }

        size_t GameEngineConfig::profileCount() const {
            return m_profiles.size();
        }

        bool GameEngineConfig::hasProfile(const std::string& name) const {
            for (size_t i = 0; i < m_profiles.size(); ++i) {
                if (m_profiles[i]->name() == name) {
                    return true;
                }
            }
            return false;
        }

        GameEngineProfile* GameEngineConfig::profile(const size_t index) const {
            assert(index < profileCount());
            return m_profiles[index].get();
        }

        void GameEngineConfig::addProfile(std::unique_ptr<GameEngineProfile> profile) {
            ensure(profile != nullptr, "profile is null");
            ensure(profile->parent() == nullptr, "profile already has a parent");
            profile->setParent(this);
            m_profiles.push_back(std::move(profile));
            profilesDidChange();
            configDidChange();
        }

        void GameEngineConfig::removeProfile(const size_t index) {
            assert(index < profileCount());
            m_profiles[index]->profileWillBeRemoved();
            kdl::vec_erase_at(m_profiles, index);
            profilesDidChange();
            configDidChange();
        }

        void GameEngineConfig::profileDidChange(GameEngineProfile*) {
            configDidChange();
        }
    }
}
