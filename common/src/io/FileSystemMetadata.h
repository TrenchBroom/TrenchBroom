/*
 Copyright (C) 2025 Kristian Duske

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
#include <iosfwd>
#include <variant>

namespace tb::io
{
namespace FileSystemMetadataKeys
{
constexpr auto ImageFilePath = "FileSystemMetadataKeys::ImageFilePath";
}

using FileSystemMetadata = std::variant<std::filesystem::path>;

std::ostream& operator<<(std::ostream& lhs, const FileSystemMetadata& rhs);

} // namespace tb::io
