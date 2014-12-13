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
    namespace View {
        
        class DetectColorRangeVisitor : public Model::ConstNodeVisitor {
        private:
            const Model::AttributeName& m_name;
            ColorRange::Type m_range;
        public:
            DetectColorRangeVisitor(const Model::AttributeName& name) :
            m_name(name),
            m_range(ColorRange::Unset) {}
            
            ColorRange::Type result() const { return m_range; }
        private:
            void doVisit(const Model::World* world)   { visitAttributableNode(world); }
            void doVisit(const Model::Layer* layer)   {}
            void doVisit(const Model::Group* group)   {}
            void doVisit(const Model::Entity* entity) { visitAttributableNode(entity); }
            void doVisit(const Model::Brush* brush)   {}
            
            void visitAttributableNode(const Model::AttributableNode* attributable) {
                static const Model::AttributeValue NullValue("");
                const Model::AttributeValue& value = attributable->attribute(m_name, NullValue);
                if (value != NullValue) {
                    const ColorRange::Type attrRange = detectColorRange(value);
                    if (m_range == ColorRange::Unset)
                        m_range = attrRange;
                    else if (m_range != attrRange)
                        m_range = ColorRange::Mixed;
                }
            }
        };
        
        ColorRange::Type detectColorRange(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) {
            DetectColorRangeVisitor visitor(name);
            Model::Node::accept(attributables.begin(), attributables.end(), visitor);
            return visitor.result();
        }
        
        ColorRange::Type detectColorRange(const StringList& components);
        
        ColorRange::Type detectColorRange(const String& str) {
            return detectColorRange(StringUtils::splitAndTrim(str, " "));
        }

        ColorRange::Type detectColorRange(const StringList& components) {
            if (components.size() != 3)
                return ColorRange::Unset;
            
            ColorRange::Type range = ColorRange::Byte;
            for (size_t i = 0; i < 3 && range == ColorRange::Byte; ++i)
                if (components[i].find('.') != String::npos)
                    range = ColorRange::Float;
            
            return range;
        }

        wxColor parseEntityColor(const String& str) {
            const StringList components = StringUtils::splitAndTrim(str, " ");
            const ColorRange::Type range = detectColorRange(components);
            assert(range != ColorRange::Mixed);

            int r = 0, g = 0, b = 0;
            if (range == ColorRange::Byte) {
                r = std::atoi(components[0].c_str());
                g = std::atoi(components[1].c_str());
                b = std::atoi(components[2].c_str());
            } else if (range == ColorRange::Float) {
                r = static_cast<int>(std::atof(components[0].c_str()) * 255.0);
                g = static_cast<int>(std::atof(components[1].c_str()) * 255.0);
                b = static_cast<int>(std::atof(components[2].c_str()) * 255.0);
            }
            
            return wxColor(r, g, b);
        }
    }
}
