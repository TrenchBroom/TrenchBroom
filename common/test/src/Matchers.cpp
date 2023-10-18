/*
 Copyright (C) 2023 Kristian Duske

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

#include "Matchers.h"

#include "kdl/string_compare.h"

namespace TrenchBroom
{

GlobMatcher::GlobMatcher(std::string glob)
  : m_glob{std::move(glob)}
{
}

bool GlobMatcher::match(const std::string& value) const
{
  return kdl::cs::str_matches_glob(value, m_glob);
}

std::string GlobMatcher::describe() const
{
  auto ss = std::stringstream{};
  ss << "matches glob \"" << m_glob << "\"";
  return ss.str();
}

GlobMatcher MatchesGlob(std::string glob)
{
  return GlobMatcher{std::move(glob)};
}

} // namespace TrenchBroom
