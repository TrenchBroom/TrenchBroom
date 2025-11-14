/*
 Copyright (C) 2010 Kristian Duske

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

#include "PathMatcher.h"

#include "kdl/path_utils.h"
#include "kdl/string_compare.h"
#include "kdl/vector_utils.h"

#include <algorithm>

namespace tb::io
{

PathMatcher makeExtensionPathMatcher(std::vector<std::filesystem::path> extensions_)
{
  return [extensions = std::move(extensions_)](
           const std::filesystem::path& path, const GetPathInfo&) {
    return std::ranges::any_of(extensions, [&](const auto& extension) {
      return kdl::path_has_extension(
        kdl::path_to_lower(path), kdl::path_to_lower(extension));
    });
  };
}

PathMatcher makeFilenamePathMatcher(std::string pattern_)
{
  return [pattern = std::move(pattern_)](
           const std::filesystem::path& path, const GetPathInfo&) {
    return kdl::ci::str_matches_glob(path.filename().string(), pattern);
  };
}

PathMatcher makePathInfoPathMatcher(std::vector<PathInfo> pathInfos_)
{
  return [pathInfos = std::move(pathInfos_)](
           const std::filesystem::path& path, const GetPathInfo& getPathInfo) {
    return kdl::vec_contains(pathInfos, getPathInfo(path));
  };
}

bool matchAnyPath(const std::filesystem::path&, const GetPathInfo&)
{
  return true;
}

} // namespace tb::io
