/*
 Copyright (C) 2021 Kristian Duske

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

#include <kdl/reflection_decl.h>
#include <kdl/result_forward.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <iosfwd>
#include <vector>

namespace TrenchBroom
{
struct Error;
} // namespace TrenchBroom

namespace TrenchBroom::Model
{
class PointTrace
{
private:
  std::vector<vm::vec3f> m_points;
  size_t m_current;

public:
  explicit PointTrace(std::vector<vm::vec3f> points);

  bool hasNextPoint() const;
  bool hasPreviousPoint() const;

  const std::vector<vm::vec3f>& points() const;
  const vm::vec3f& currentPoint() const;
  const vm::vec3f currentDirection() const;

  void advance();
  void retreat();

  kdl_reflect_decl(PointTrace, m_points, m_current);
};

kdl::result<PointTrace, Error> loadPointFile(std::istream& stream);
} // namespace TrenchBroom::Model
