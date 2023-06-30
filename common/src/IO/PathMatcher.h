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

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{

enum class PathInfo;

using GetPathInfo = std::function<PathInfo(const std::filesystem::path&)>;
using PathMatcher = std::function<bool(const std::filesystem::path&, const GetPathInfo&)>;

PathMatcher makeExtensionPathMatcher(std::vector<std::string> extensions);
PathMatcher makeFilenamePathMatcher(std::string filename);
PathMatcher makePathInfoPathMatcher(std::vector<PathInfo> pathInfos);

bool matchAnyPath(const std::filesystem::path& path, const GetPathInfo& getPathInfo);

} // namespace TrenchBroom::IO
