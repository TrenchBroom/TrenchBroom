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
// that includes these operators for the vecmath types to ensure that they
// work consistently.

// Include this header instead of <catch2/catch.hpp> to ensure that vecmath
// stream operators work consistently.

#include <vecmath/forward.h>
#include <vecmath/bbox_io.h>
#include <vecmath/line_io.h>
#include <vecmath/mat_io.h>
#include <vecmath/plane_io.h>
#include <vecmath/ray_io.h>
#include <vecmath/vec_io.h>

#include <catch2/catch.hpp>

