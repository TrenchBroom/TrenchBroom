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

#pragma once

#include "StringMakers.h"
#include "mdl/CatchConfig.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/UvAttributes.h"

#include <cassert>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
class Brush;
class BrushFace;
class Node;

class BrushVertexMatcher : public Catch::Matchers::MatcherBase<Brush>
{
  const Brush& m_expected;
  double m_epsilon;

public:
  BrushVertexMatcher(const Brush& expected, double epsilon);

  bool match(const Brush& in) const override;

  std::string describe() const override;
};

BrushVertexMatcher MatchesBrushVertices(const Brush& expected, double epsilon);

class NodeMatcher : public Catch::Matchers::MatcherBase<Node>
{
  const Node& m_expected;

public:
  explicit NodeMatcher(const Node& expected);

  bool match(const Node& in) const override;

  std::string describe() const override;
};

NodeMatcher MatchesNode(const Node& expected);

class BrushFaceAttributesMatcher : public Catch::Matchers::MatcherBase<BrushFace>
{
private:
  std::string m_expectedMaterialName;
  UvAttributes m_expectedUvAttributes;
  SurfaceAttributes m_expectedSurfaceAttributes;

public:
  BrushFaceAttributesMatcher(
    std::string expectedMaterialName,
    UvAttributes expectedUvAttributes,
    SurfaceAttributes expectedSurfaceAttributes);

  bool match(const BrushFace& in) const override;

  std::string describe() const override;
};

BrushFaceAttributesMatcher MatchesBrushFaceAttributes(
  std::string expectedMaterialName,
  UvAttributes expectedUvAttributes,
  SurfaceAttributes expectedSurfaceAttributes);

/**
 * Matches the material name and the UV and surface attributes of the given face. They are
 * copied, so the matcher is unaffected by later changes to the face.
 */
BrushFaceAttributesMatcher MatchesBrushFaceAttributes(const BrushFace& expected);

class UpdateBrushFaceAttributesMatcher
  : public Catch::Matchers::MatcherBase<UpdateBrushFaceAttributes>
{
  UpdateBrushFaceAttributes m_expected;

public:
  explicit UpdateBrushFaceAttributesMatcher(UpdateBrushFaceAttributes expected);

  bool match(const UpdateBrushFaceAttributes& in) const override;

  std::string describe() const override;
};

UpdateBrushFaceAttributesMatcher MatchesUpdateBrushFaceAttributes(
  UpdateBrushFaceAttributes expected);

} // namespace tb::mdl
