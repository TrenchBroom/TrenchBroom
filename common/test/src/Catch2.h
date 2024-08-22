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

#pragma once

// The catch2 header must be included only when all stream insertion
// operators used in assertions are visible. We add this new wrapper header
// that includes these operators for the vm types to ensure that they
// work consistently.

// Include this header instead of <catch2/catch.hpp> to ensure that vm
// stream operators work consistently.

#include "kdl/result_io.h"

#include "vm/bbox_io.h"
#include "vm/forward.h"
#include "vm/line_io.h"
#include "vm/mat_io.h"
#include "vm/plane_io.h"
#include "vm/ray_io.h"
#include "vm/vec_io.h"

#define CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS 1
#include <catch2/catch.hpp>
