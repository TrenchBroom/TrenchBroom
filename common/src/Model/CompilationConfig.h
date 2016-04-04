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

#ifndef CompilationConfig_h
#define CompilationConfig_h

#include "Model/CompilationProfile.h"

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig {
        private:
            CompilationProfile::List m_profiles;
        public:
            CompilationConfig(const CompilationProfile::List& profiles);
            
            size_t profileCount() const;
            const CompilationProfile& profile(size_t index);
            
            void addProfile(const CompilationProfile& profile);
            void updateProfile(size_t index, const CompilationProfile& profile);
            void removeProfile(size_t index);
        };
    }
}

#endif /* CompilationConfig_h */
