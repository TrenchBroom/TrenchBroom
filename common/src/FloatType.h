/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <vecmath/constants.h>
#include <vecmath/forward.h>

using FloatType = double;

namespace vm {
    using vec3 = vm::vec<FloatType,3>;
    using vec2 = vm::vec<FloatType,2>;
    using mat4x4 = vm::mat<FloatType,4,4>;
    using quat3 = vm::quat<FloatType>;
    using line3 = vm::line<FloatType,3>;
    using ray3 = vm::ray<FloatType,3>;
    using segment3 = vm::segment<FloatType,3>;
    using plane3 = vm::plane<FloatType,3>;
    using polygon3 = vm::polygon<FloatType,3>;
    using bbox3 = vm::bbox<FloatType,3>;
    using bbox2 = vm::bbox<FloatType,2>;

    using C = constants<FloatType>;
}

#endif
