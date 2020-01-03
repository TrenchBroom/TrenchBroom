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

#ifndef TrenchBroom_EntityColor
#define TrenchBroom_EntityColor

#include <string>
#include <vector>

namespace TrenchBroom {
    class Color;

    namespace Assets {
        namespace ColorRange {
            using Type = int;
        }
    }

    namespace Model {
        class AttributableNode;

        Assets::ColorRange::Type detectColorRange(const std::string& name, const std::vector<AttributableNode*>& attributables);

        const std::string convertEntityColor(const std::string& str, Assets::ColorRange::Type colorRange);
        Color parseEntityColor(const std::string& str);
        std::string entityColorAsString(const Color& color, Assets::ColorRange::Type colorRange);
    }
}

#endif /* defined(TrenchBroom_EntityColor) */
