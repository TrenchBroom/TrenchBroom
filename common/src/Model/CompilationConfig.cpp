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

#include "CompilationConfig.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Model {
        CompilationConfig::CompilationConfig(const CompilationProfile::List& profiles) :
        m_profiles(profiles) {}

        size_t CompilationConfig::profileCount() const {
            return m_profiles.size();
        }
        
        const CompilationProfile& CompilationConfig::profile(const size_t index) {
            assert(index < profileCount());
            return m_profiles[index];
        }
        
        void CompilationConfig::addProfile(const CompilationProfile& profile) {
            m_profiles.push_back(profile);
        }
        
        void CompilationConfig::updateProfile(const size_t index, const CompilationProfile& profile) {
            assert(index < profileCount());
            m_profiles[index] = profile;
        }
        
        void CompilationConfig::removeProfile(const size_t index) {
            assert(index < profileCount());
            VectorUtils::erase(m_profiles, index);
        }
    }
}
