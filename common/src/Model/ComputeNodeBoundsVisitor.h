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

#ifndef __TrenchBroom__ComputeNodeBoundsVisitor__
#define __TrenchBroom__ComputeNodeBoundsVisitor__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class ComputeNodeBoundsVisitor : public ConstNodeVisitor {
        private:
            bool m_initialized;
        public:
            BBox3 m_bounds;
            ComputeNodeBoundsVisitor(const BBox3& defaultBounds);
            const BBox3& bounds() const;
        private:
            void doVisit(const World* world);
            void doVisit(const Layer* layer);
            void doVisit(const Group* group);
            void doVisit(const Entity* entity);
            void doVisit(const Brush* brush);
            void mergeWith(const BBox3& bounds);
        };
    }
}


#endif /* defined(__TrenchBroom__ComputeNodeBoundsVisitor__) */
