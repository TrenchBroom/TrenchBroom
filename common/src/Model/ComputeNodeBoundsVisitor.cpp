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

#include "ComputeNodeBoundsVisitor.h"

#include "Model/Brush.h"
#include "Model/Group.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        ComputeNodeBoundsVisitor::ComputeNodeBoundsVisitor(const BoundsType type, const vm::bbox3& defaultBounds) :
        m_initialized(false),
        m_boundsType(type),
        m_defaultBounds(defaultBounds) {}

        const vm::bbox3& ComputeNodeBoundsVisitor::bounds() const {
            if (m_builder.initialized()) {
                return m_builder.bounds();
            } else {
                return m_defaultBounds;
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const World* world) {}
        void ComputeNodeBoundsVisitor::doVisit(const Layer* layer) {}

        void ComputeNodeBoundsVisitor::doVisit(const Group* group) {
            if (m_boundsType == BoundsType::Culling) {
                m_builder.add(group->cullingBounds());
            } else {
                m_builder.add(group->bounds());
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const Entity* entity) {
            if (m_boundsType == BoundsType::Culling) {
                m_builder.add(entity->cullingBounds());
            } else {
                m_builder.add(entity->bounds());
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const Brush* brush) {
            if (m_boundsType == BoundsType::Culling) {
                m_builder.add(brush->cullingBounds());
            } else {
                m_builder.add(brush->bounds());
            }
        }

        vm::bbox3 computeBounds(const Model::NodeList& nodes) {
            return computeBounds(std::begin(nodes), std::end(nodes));
        }

        vm::bbox3 computeCullingBounds(const Model::NodeList& nodes) {
            return computeCullingBounds(std::begin(nodes), std::end(nodes));
        }
    }
}
