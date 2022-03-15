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

#include "PointTrace.h"

#include "Ensure.h"

#include <kdl/reflection_impl.h>
#include <kdl/string_utils.h>

#include <vecmath/distance.h>
#include <vecmath/ray.h>
#include <vecmath/vec_io.h>

#include <cassert>
#include <istream>

namespace TrenchBroom::Model {

PointTrace::PointTrace(std::vector<vm::vec3f> points)
  : m_points{std::move(points)}
  , m_current{0} {
  ensure(!m_points.empty(), "Point trace is not empty");
}

bool PointTrace::hasNextPoint() const {
  return m_current < m_points.size() - 1;
}

bool PointTrace::hasPreviousPoint() const {
  return m_current > 0;
}

const std::vector<vm::vec3f>& PointTrace::points() const {
  return m_points;
}

const vm::vec3f& PointTrace::currentPoint() const {
  return m_points[m_current];
}

const vm::vec3f PointTrace::currentDirection() const {
  if (m_points.size() <= 1) {
    return vm::vec3f::pos_x();
  } else if (m_current >= m_points.size() - 1) {
    return vm::normalize(m_points[m_points.size() - 1] - m_points[m_points.size() - 2]);
  } else {
    return vm::normalize(m_points[m_current + 1] - m_points[m_current]);
  }
}

void PointTrace::advance() {
  if (hasNextPoint()) {
    ++m_current;
  }
}

void PointTrace::retreat() {
  if (hasPreviousPoint()) {
    --m_current;
  }
}

static std::vector<vm::vec3f> smoothPoints(const std::vector<vm::vec3f>& points) {
  assert(points.size() > 1);

  auto result = std::vector<vm::vec3f>{points[0]};

  auto it = std::find_if(std::next(std::begin(points)), std::end(points), [&](const auto& p) {
    return p != points[0];
  });

  if (it == std::end(points)) {
    return result;
  }

  result.push_back(*it);
  ++it;

  auto ray = vm::ray3f{result[0], vm::normalize(result[1] - result[0])};
  while (it != std::end(points)) {
    const auto& cur = *it;
    const auto dist = vm::squared_distance(ray, cur).distance;
    if (dist > 1.0f) {
      ray = vm::ray3f{result.back(), vm::normalize(cur - result.back())};
      result.push_back(cur);
    } else {
      result.back() = cur;
    }
    ++it;
  }

  assert(result.size() > 1);
  return result;
}

static std::vector<vm::vec3f> segmentizePoints(const std::vector<vm::vec3f>& points) {
  auto segmentizedPoints = std::vector<vm::vec3f>{};
  if (points.size() > 1) {
    for (size_t i = 0; i < points.size() - 1; ++i) {
      const auto& curPoint = points[i];
      const auto& nextPoint = points[i + 1];
      const auto dir = vm::normalize(nextPoint - curPoint);

      segmentizedPoints.push_back(curPoint);
      const auto dist = length(nextPoint - curPoint);
      const auto segments = static_cast<size_t>(dist / 64.0f);
      for (unsigned int j = 1; j < segments; ++j) {
        segmentizedPoints.push_back(curPoint + dir * static_cast<float>(j) * 64.0f);
      }
    }
    segmentizedPoints.push_back(points.back());
  }
  return segmentizedPoints;
}

kdl_reflect_impl(PointTrace);

std::optional<PointTrace> loadPointFile(std::istream& stream) {
  const auto str = std::string{std::istreambuf_iterator<char>{stream}, {}};

  auto points = std::vector<vm::vec3f>{};
  vm::parse_all<float, 3>(str, std::back_inserter(points));

  if (points.size() < 2) {
    return std::nullopt;
  }

  points = smoothPoints(points);
  if (points.size() < 2) {
    return std::nullopt;
  }

  points = segmentizePoints(points);
  return PointTrace{std::move(points)};
}

} // namespace TrenchBroom::Model
