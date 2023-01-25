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

#pragma once

#include "Model/CompilationProfile.h"

#include <iosfwd>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class CompilationConfig
{
private:
  std::vector<CompilationProfile> m_profiles;

public:
  CompilationConfig();
  explicit CompilationConfig(std::vector<CompilationProfile> profiles);

  friend bool operator==(const CompilationConfig& lhs, const CompilationConfig& rhs);
  friend bool operator!=(const CompilationConfig& lhs, const CompilationConfig& rhs);
  friend std::ostream& operator<<(std::ostream& str, const CompilationConfig& config);

  const std::vector<CompilationProfile>& profiles() const;
  size_t profileCount() const;
  size_t indexOfProfile(const CompilationProfile& profile) const;
  CompilationProfile& profile(size_t index);
  const CompilationProfile& profile(size_t index) const;

  void addProfile(CompilationProfile profile);
  void removeProfile(size_t index);
};
} // namespace Model
} // namespace TrenchBroom
