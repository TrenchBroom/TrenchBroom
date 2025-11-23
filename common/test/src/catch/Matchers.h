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

#include <cassert>
#include <vector>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
class Node;

class NodeMatcher : public Catch::Matchers::MatcherBase<Node>
{
  const Node& m_expected;

public:
  explicit NodeMatcher(const Node& expected);

  bool match(const Node& in) const override;

  std::string describe() const override;
};

NodeMatcher MatchesNode(const Node& expected);

class NodeVectorMatcher : public Catch::Matchers::MatcherBase<const std::vector<Node*>&>
{
  const std::vector<Node*> m_expected;

public:
  explicit NodeVectorMatcher(std::vector<Node*> expected);

  bool match(const std::vector<Node*>& in) const override;

  std::string describe() const override;
};

NodeMatcher MatchesNodeVector(std::vector<Node*> expected);

} // namespace tb::mdl
