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

#include "LockState.h"

#include "Macros.h"

#include <iostream>

namespace TrenchBroom {
    namespace Model {
        std::ostream& operator<<(std::ostream& str, const LockState state) {
            switch (state) {
                case LockState::Inherited:
                    str << "Inherited";
                    break;
                case LockState::Locked:
                    str << "Locked";
                    break;
                case LockState::Unlocked:
                    str << "Unlocked";
                    break;
                switchDefault()
            }
            return str;
        }
    }
}

