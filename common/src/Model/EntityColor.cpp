/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class DetectColorRangeVisitor : public ConstNodeVisitor {
        private:
            const AttributeName& m_name;
            Assets::ColorRange::Type m_range;
        public:
            DetectColorRangeVisitor(const AttributeName& name) :
            m_name(name),
            m_range(Assets::ColorRange::Unset) {}
            
            Assets::ColorRange::Type result() const { return m_range; }
        private:
            void doVisit(const World* world)   { visitAttributableNode(world); }
            void doVisit(const Layer* layer)   {}
            void doVisit(const Group* group)   {}
            void doVisit(const Entity* entity) { visitAttributableNode(entity); }
            void doVisit(const Brush* brush)   {}
            
            void visitAttributableNode(const AttributableNode* attributable) {
                static const AttributeValue NullValue("");
                const AttributeValue& value = attributable->attribute(m_name, NullValue);
                if (value != NullValue) {
                    const Assets::ColorRange::Type attrRange = Assets::detectColorRange(value);
                    if (m_range == Assets::ColorRange::Unset)
                        m_range = attrRange;
                    else if (m_range != attrRange)
                        m_range = Assets::ColorRange::Mixed;
                }
            }
        };
        
        Assets::ColorRange::Type detectColorRange(const AttributeName& name, const AttributableNodeList& attributables) {
            DetectColorRangeVisitor visitor(name);
            Node::accept(attributables.begin(), attributables.end(), visitor);
            return visitor.result();
        }
        
        const String convertEntityColor(const String& str, const Assets::ColorRange::Type colorRange) {
            const wxColor color = parseEntityColor(str);
            return entityColorAsString(color, colorRange);
        }

        wxColor parseEntityColor(const String& str) {
            const StringList components = StringUtils::splitAndTrim(str, " ");
            const Assets::ColorRange::Type range = Assets::detectColorRange(components);
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
            
            return wxColor(r, g, b);
        }

        String entityColorAsString(const wxColor& color, const Assets::ColorRange::Type colorRange) {
            StringStream result;
            if (colorRange == Assets::ColorRange::Byte) {
                result << int(color.Red()) << " " << int(color.Green()) << " " << int(color.Blue());
            } else if (colorRange == Assets::ColorRange::Float) {
                result << float(color.Red()) / 255.0f << " " << float(color.Green()) / 255.0f << " "<< float(color.Blue()) / 255.0f;
            }
            return result.str();
        }
    }
}
