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

#ifndef __TrenchBroom__Snapshot__
#define __TrenchBroom__Snapshot__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Snapshot {
        private:
            typedef std::vector<Model::EntitySnapshot> EntitySnapshotList;
            typedef std::vector<Model::BrushSnapshot> BrushSnapshotList;
            typedef std::vector<Model::BrushFaceSnapshot> BrushFaceSnapshotList;
            
            class Builder : public ObjectVisitor {
            private:
                EntitySnapshotList& m_entitySnapshots;
                BrushSnapshotList& m_brushSnapshots;
            public:
                Builder(EntitySnapshotList& entitySnapshots, BrushSnapshotList& brushSnapshots);
                
                void doVisit(Entity* entity);
                void doVisit(Brush* brush);
            };

            EntitySnapshotList m_entitySnapshots;
            BrushSnapshotList m_brushSnapshots;
            BrushFaceSnapshotList m_faceSnapshots;
        public:
            Snapshot();
            Snapshot(const Model::ObjectList& objects);
            Snapshot(const Model::EntityList& entities);
            Snapshot(const Model::BrushList& brushes);
            Snapshot(const Model::BrushFaceList& faces);
            void restore(const BBox3& worldBounds);
        };
    }
}

#endif /* defined(__TrenchBroom__Snapshot__) */
