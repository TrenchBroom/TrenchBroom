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

#include "TransformObjectVisitor.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        TransformObjectVisitor::TransformObjectVisitor(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) :
        m_transformation(transformation),
        m_lockTextures(lockTextures),
        m_worldBounds(worldBounds) {}

        void TransformObjectVisitor::doVisit(World*)         {}
        void TransformObjectVisitor::doVisit(Layer*)         {}
        void TransformObjectVisitor::doVisit(Group* group)   {  group->transform(m_transformation, m_lockTextures, m_worldBounds); }
        void TransformObjectVisitor::doVisit(Entity* entity) { entity->transform(m_transformation, m_lockTextures, m_worldBounds); }
        void TransformObjectVisitor::doVisit(BrushNode* brush)   {  brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
    }
}
