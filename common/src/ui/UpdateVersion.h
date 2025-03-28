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

#include "upd/Asset.h"

#include "kdl/reflection_decl.h"

#include <iosfwd>
#include <variant>

namespace tb::ui
{
struct SemanticVersion
{
  int major;
  int minor;
  int patch;
  std::optional<int> rc = std::nullopt;

  kdl_reflect_decl(SemanticVersion, major, minor, patch, rc);
};

struct TemporalVersion
{
  int year;
  int no;
  std::optional<int> rc = std::nullopt;

  kdl_reflect_decl(TemporalVersion, year, no, rc);
};

using UpdateVersion = std::variant<SemanticVersion, TemporalVersion>;

std::partial_ordering operator<=>(const UpdateVersion& lhs, const UpdateVersion& rhs);
bool operator==(const UpdateVersion& lhs, const UpdateVersion& rhs);
std::ostream& operator<<(std::ostream& lhs, const UpdateVersion& rhs);

std::optional<UpdateVersion> parseUpdateVersion(const QString& tag);
QString describeUpdateVersion(const UpdateVersion& version);

std::optional<upd::Asset> chooseAsset(const QList<upd::Asset>& assets);

} // namespace tb::ui
