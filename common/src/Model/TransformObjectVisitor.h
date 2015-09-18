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

#ifndef TrenchBroom_TransformObjectVisitor
#define TrenchBroom_TransformObjectVisitor

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class TransformObjectVisitor : public NodeVisitor {
        private:
            const Mat4x4d& m_transformation;
            bool m_lockTextures;
            const BBox3& m_worldBounds;
        public:
            TransformObjectVisitor(const Mat4x4d& transformation, bool lockTextures, const BBox3& worldBounds);
        private:
            void doVisit(World* world);
            void doVisit(Layer* layer);
            void doVisit(Group* group);
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectVisitor) */
