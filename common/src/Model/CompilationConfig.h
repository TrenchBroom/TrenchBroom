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

#ifndef CompilationConfig_h
#define CompilationConfig_h

#include "Notifier.h"
#include "Model/CompilationProfile.h"

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig {
        private:
            CompilationProfile::List m_profiles;
			size_t m_lastUsedProfileIndex; // addition 12-29-18-03 Remember last selection index -Qmaster
        public:
            mutable Notifier0 profilesDidChange;
        public:
            CompilationConfig();
            CompilationConfig(const CompilationProfile::List& profiles);
			CompilationConfig(const CompilationProfile::List& profiles, size_t index);
            CompilationConfig(const CompilationConfig& other);
            ~CompilationConfig();
            
            CompilationConfig& operator=(CompilationConfig other);
            friend void swap(CompilationConfig& lhs, CompilationConfig& rhs);
            
            size_t profileCount() const;
            CompilationProfile* profile(size_t index) const;
            
            void addProfile(CompilationProfile* profile);
            void removeProfile(size_t index);

			// Remember last selection index this session -Qmaster
			size_t getLastUsedProfileIndex() const;
			void setLastUsedProfileIndex(size_t index); 
        };
    }
}

#endif /* CompilationConfig_h */
