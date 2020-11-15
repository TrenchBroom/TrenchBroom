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

#include "EntityColor.h"

#include "Color.h"
#include "Model/AttributableNode.h"
#include "Assets/ColorRange.h"
#include "Model/EntityNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/string_utils.h>

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        Assets::ColorRange::Type detectColorRange(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            auto result = Assets::ColorRange::Unset;
            for (auto* attributable : attributables) {
                attributable->accept(kdl::overload(
                    [&](const AttributableNode* node) {
                        if (const auto* value = node->entity().attribute(name)) {
                            const auto range = Assets::detectColorRange(*value);
                            if (result == Assets::ColorRange::Unset) {
                                result = range;
                            } else if (result != range) {
                                result = Assets::ColorRange::Mixed;
                            }
                        }
                    },
                    [](const LayerNode*) {},
                    [](const GroupNode*) {},
                    [](const BrushNode*) {}
                ));
            }
            return result;
        }

        const std::string convertEntityColor(const std::string& str, const Assets::ColorRange::Type colorRange) {
            const auto color = parseEntityColor(str);
            return entityColorAsString(color, colorRange);
        }

        Color parseEntityColor(const std::string& str) {
            const auto components = kdl::str_split(str, " ");
            const auto range = Assets::detectColorRange(components);
            assert(range != Assets::ColorRange::Mixed);

            int r = 0, g = 0, b = 0;
            if (range == Assets::ColorRange::Byte) {
                r = std::atoi(components[0].c_str());
                g = std::atoi(components[1].c_str());
                b = std::atoi(components[2].c_str());
            } else if (range == Assets::ColorRange::Float) {
                r = static_cast<int>(std::atof(components[0].c_str()) * 255.0);
                g = static_cast<int>(std::atof(components[1].c_str()) * 255.0);
                b = static_cast<int>(std::atof(components[2].c_str()) * 255.0);
            }

            return Color(r, g, b);
        }

        std::string entityColorAsString(const Color& color, const Assets::ColorRange::Type colorRange) {
            std::stringstream result;
            if (colorRange == Assets::ColorRange::Byte) {
                result << int(color.r() * 255.0f) << " " << int(color.g() * 255.0f) << " " << int(color.b() * 255.0f);
            } else if (colorRange == Assets::ColorRange::Float) {
                result << float(color.r()) << " " << float(color.g()) << " " << float(color.b());
            }
            return result.str();
        }
    }
}
