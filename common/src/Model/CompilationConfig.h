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

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;

        class CompilationConfig {
        private:
            std::vector<std::unique_ptr<CompilationProfile>> m_profiles;
        public:
            CompilationConfig();
            explicit CompilationConfig(std::vector<std::unique_ptr<CompilationProfile>> profiles);
            CompilationConfig(const CompilationConfig& other);
            ~CompilationConfig();

            CompilationConfig& operator=(CompilationConfig other);
            friend void swap(CompilationConfig& lhs, CompilationConfig& rhs);
            bool operator==(const CompilationConfig& other) const;

            size_t profileCount() const;
            CompilationProfile* profile(size_t index) const;
            size_t indexOfProfile(CompilationProfile* profile) const;

            void addProfile(std::unique_ptr<CompilationProfile> profile);
            void removeProfile(size_t index);
        };
    }
}

#endif /* CompilationConfig_h */
