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
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>

namespace TrenchBroom {
    namespace Model {
        TransformObjectVisitor::TransformObjectVisitor(const vm::bbox3& worldBounds, const vm::mat4x4& transformation,  const bool lockTextures) :
        m_worldBounds(worldBounds),
        m_transformation(transformation),
        m_lockTextures(lockTextures) {}

        const std::optional<TransformError>& TransformObjectVisitor::error() const {
            return m_error;
        }

        void TransformObjectVisitor::doVisit(WorldNode*)         {}
        void TransformObjectVisitor::doVisit(LayerNode*)         {}
        void TransformObjectVisitor::doVisit(GroupNode* group)   { transform(group); }
        void TransformObjectVisitor::doVisit(EntityNode* entity) { transform(entity); }
        void TransformObjectVisitor::doVisit(BrushNode* brush)   { transform(brush); }

        void TransformObjectVisitor::transform(Object* object) {
            object->transform(m_worldBounds, m_transformation, m_lockTextures)
                .visit(kdl::overload {
                    []() {},
                    [&](TransformError&& e) {
                        m_error = std::move(e);
                        cancel();
                    },
                });
        }
    }
}
