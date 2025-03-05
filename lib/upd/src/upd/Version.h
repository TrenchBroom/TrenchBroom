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

#include <QString>

#include <functional>
#include <optional>

namespace upd
{

/**
 * A function that parses a git tag name into a version. Returns std::nullopt if the tag
 * cannot be parsed.
 */
template <typename Version>
using ParseVersion = std::function<std::optional<Version>(const QString&)>;

/**
 * A function that describes a version. Returns a human readable string representation of
 * the version.
 */
template <typename Version>
using DescribeVersion = std::function<QString(const Version&)>;

} // namespace upd
