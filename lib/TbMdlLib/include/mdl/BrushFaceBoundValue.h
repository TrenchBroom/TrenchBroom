/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"

namespace tb::mdl
{
class BrushFaceHandle;
class Map;

/**
 * The search/filter query language's BoundValue for a brush face -- `material`,
 * `normal`, `bounds`/`center` of its polygon, `entity` (the owning entity), `tags`
 * (this face's own tags) and `selected` (per-face selection), plus `layerName`/
 * `groupName`/`visible`/`locked` inherited from the owning BrushNode's containment.
 */
el::BoundValue makeBrushFaceBoundValue(const Map& map, const BrushFaceHandle& handle);

} // namespace tb::mdl
