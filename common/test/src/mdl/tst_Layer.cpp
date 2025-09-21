/*
 Copyright (C) 2020 Kristian Duske

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

#include "mdl/Layer.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("Layer")
{
  SECTION("sortIndex")
  {
    SECTION("Regular layer")
    {
      auto layer = Layer{"defaultLayer", false};
      CHECK(layer.sortIndex() != Layer::defaultLayerSortIndex());

      layer.setSortIndex(Layer::defaultLayerSortIndex() + 1);
      CHECK(layer.sortIndex() == Layer::defaultLayerSortIndex() + 1);
    }

    SECTION("Default layer")
    {
      auto layer = Layer{"defaultLayer", true};
      CHECK(layer.sortIndex() == Layer::defaultLayerSortIndex());

      layer.setSortIndex(Layer::defaultLayerSortIndex() + 1);
      CHECK(layer.sortIndex() == Layer::defaultLayerSortIndex());
    }
  }
}

} // namespace tb::mdl
