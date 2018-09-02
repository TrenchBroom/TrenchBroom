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
        ComputeNodeBoundsVisitor::ComputeNodeBoundsVisitor(const bbox3& defaultBounds) :
        m_initialized(false),
        m_bounds(defaultBounds) {}
        
        const bbox3& ComputeNodeBoundsVisitor::bounds() const {
            return m_bounds;
        }

        void ComputeNodeBoundsVisitor::doVisit(const World* world) {}
        void ComputeNodeBoundsVisitor::doVisit(const Layer* layer) {}
        
        void ComputeNodeBoundsVisitor::doVisit(const Group* group) {
            mergeWith(group->bounds());
        }
        
        void ComputeNodeBoundsVisitor::doVisit(const Entity* entity) {
            mergeWith(entity->bounds());
        }
        
        void ComputeNodeBoundsVisitor::doVisit(const Brush* brush) {
            mergeWith(brush->bounds());
        }

        void ComputeNodeBoundsVisitor::mergeWith(const bbox3& bounds) {
            if (!m_initialized) {
                m_bounds = bounds;
                m_initialized = true;
            } else {
                m_bounds = merge(m_bounds, bounds);
            }
        }

        bbox3 computeBounds(const Model::NodeList& nodes) {
            return computeBounds(std::begin(nodes), std::end(nodes));
        }
    }
}
