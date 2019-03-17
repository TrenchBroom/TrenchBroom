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

#include "IntersectNodeWithRayVisitor.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        IntersectNodeWithRayVisitor::IntersectNodeWithRayVisitor(const vm::ray3& ray) :
        NodeQuery(vm::nan<FloatType>()),
        m_ray(ray) {}

        void IntersectNodeWithRayVisitor::doVisit(World* world)   {}
        void IntersectNodeWithRayVisitor::doVisit(Layer* layer)   {}
        void IntersectNodeWithRayVisitor::doVisit(Group* group)   { setResult(group->intersectWithRay(m_ray)); }
        void IntersectNodeWithRayVisitor::doVisit(Entity* entity) { setResult(entity->intersectWithRay(m_ray)); }
        void IntersectNodeWithRayVisitor::doVisit(Brush* brush)   { setResult(brush->intersectWithRay(m_ray)); }

        FloatType IntersectNodeWithRayVisitor::doCombineResults(FloatType oldDistance, FloatType newDistance) const {
            if (vm::isnan(oldDistance)) {
                return newDistance;
            } else if (vm::isnan(newDistance)) {
                return oldDistance;
            } else {
                return vm::min(oldDistance, newDistance);
            }
        }
    }
}
