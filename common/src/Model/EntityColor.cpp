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
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

#include <kdl/string_utils.h>

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class DetectColorRangeVisitor : public ConstNodeVisitor {
        private:
            const std::string& m_name;
            Assets::ColorRange::Type m_range;
        public:
            DetectColorRangeVisitor(const std::string& name) :
            m_name(name),
            m_range(Assets::ColorRange::Unset) {}

            Assets::ColorRange::Type result() const { return m_range; }
        private:
            void doVisit(const World* world) override   { visitAttributableNode(world); }
            void doVisit(const Layer*) override         {}
            void doVisit(const Group*) override         {}
            void doVisit(const Entity* entity) override { visitAttributableNode(entity); }
            void doVisit(const BrushNode*) override         {}

            void visitAttributableNode(const AttributableNode* attributable) {
                static const auto NullValue("");
                const auto& value = attributable->attribute(m_name, NullValue);
                if (value != NullValue) {
                    const Assets::ColorRange::Type attrRange = Assets::detectColorRange(value);
                    if (m_range == Assets::ColorRange::Unset)
                        m_range = attrRange;
                    else if (m_range != attrRange)
                        m_range = Assets::ColorRange::Mixed;
                }
            }
        };

        Assets::ColorRange::Type detectColorRange(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            DetectColorRangeVisitor visitor(name);
            Node::accept(std::begin(attributables), std::end(attributables), visitor);
            return visitor.result();
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
