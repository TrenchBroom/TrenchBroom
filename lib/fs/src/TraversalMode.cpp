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

#include "TraversalMode.h"

#include "kdl/reflection_impl.h"

namespace tb::io
{

const TraversalMode TraversalMode::Flat = TraversalMode{0};
const TraversalMode TraversalMode::Recursive = TraversalMode{};

kdl_reflect_impl(TraversalMode);

std::optional<TraversalMode> TraversalMode::reduceDepth(size_t depthToSubtract) const
{
  return !depth ? std::optional{*this}
         : *depth >= depthToSubtract
           ? std::optional{TraversalMode{*depth - depthToSubtract}}
           : std::nullopt;
}


} // namespace tb::io
