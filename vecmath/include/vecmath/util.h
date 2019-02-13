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

#ifndef TRENCHBROOM_UTIL_H
#define TRENCHBROOM_UTIL_H

#include <cstddef>

namespace vm {
    enum class side {
        front,
        back,
        both
    };

    enum class direction {
        forward,
        backward,
        left,
        right,
        up,
        down
    };

    enum class rotation_axis {
        roll,
        pitch,
        yaw

    };

    enum class point_status {
        above,
        below,
        inside
    };

    namespace axis {
        using type = size_t;
        static const type x = 0;
        static const type y = 1;
        static const type z = 2;
    }
}

#endif //TRENCHBROOM_UTIL_H
