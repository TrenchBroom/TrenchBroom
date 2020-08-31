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

#include "CompilationConfig.h"

#include "Ensure.h"
#include "Model/CompilationProfile.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        CompilationConfig::CompilationConfig() {}

        CompilationConfig::CompilationConfig(std::vector<std::unique_ptr<CompilationProfile>> profiles) :
        m_profiles(std::move(profiles)) {
            for (auto& profile : m_profiles) {
                ensure(profile->parent() == nullptr, "profile already has a parent");
                profile->setParent(this);
            }
        }

        CompilationConfig::CompilationConfig(const CompilationConfig& other) {
            m_profiles.reserve(other.m_profiles.size());

            for (const auto& original : other.m_profiles) {
                std::unique_ptr<CompilationProfile> clone = original->clone();
                ensure(clone->parent() == nullptr, "profile already has a parent");
                clone->setParent(this);
                m_profiles.push_back(std::move(clone));
            }
        }

        CompilationConfig::~CompilationConfig() = default;

        CompilationConfig& CompilationConfig::operator=(CompilationConfig other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(CompilationConfig& lhs, CompilationConfig& rhs) {
            using std::swap;
            swap(lhs.m_profiles, rhs.m_profiles);
            swap(lhs.profilesDidChange, rhs.profilesDidChange);
            swap(lhs.configDidChange, rhs.configDidChange);
        }

        size_t CompilationConfig::profileCount() const {
            return m_profiles.size();
        }

        CompilationProfile* CompilationConfig::profile(const size_t index) const {
            assert(index < profileCount());
            return m_profiles[index].get();
        }

        size_t CompilationConfig::indexOfProfile(CompilationProfile* profile) const {
            auto result = kdl::vec_index_of(m_profiles, [=](const auto& ptr){
                return ptr.get() == profile;
            });
            assert(result.has_value());
            return result.value();
        }

        void CompilationConfig::addProfile(std::unique_ptr<CompilationProfile> profile) {
            ensure(profile != nullptr, "profile is null");
            ensure(profile->parent() == nullptr, "profile already has a parent");
            profile->setParent(this);
            m_profiles.push_back(std::move(profile));
            profilesDidChange();
            configDidChange();
        }

        void CompilationConfig::removeProfile(const size_t index) {
            assert(index < profileCount());
            m_profiles[index]->profileWillBeRemoved();
            kdl::vec_erase_at(m_profiles, index);
            profilesDidChange();
            configDidChange();
        }
    }
}
