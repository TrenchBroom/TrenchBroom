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

#include "Snapshot.h"

#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        Snapshot::Snapshot() {}

        Snapshot::Snapshot(const Model::ObjectList& objects) {
            Model::ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object& object = **it;
                if (object.type() == Model::Object::Type_Entity)
                    m_entitySnapshots.push_back(static_cast<Model::Entity&>(object).takeSnapshot());
                else if (object.type() == Model::Object::Type_Brush)
                    m_brushSnapshots.push_back(static_cast<Model::Brush&>(object).takeSnapshot());
            }
        }

        Snapshot::Snapshot(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity& entity = **it;
                m_entitySnapshots.push_back(entity.takeSnapshot());
            }
        }
        
        Snapshot::Snapshot(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush& brush = **it;
                m_brushSnapshots.push_back(brush.takeSnapshot());
            }
        }

        Snapshot::Snapshot(const Model::BrushFaceList& faces) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace& face = **it;
                m_faceSnapshots.push_back(face.takeSnapshot());
            }
        }

        void Snapshot::restore(const BBox3& worldBounds) {
            EntitySnapshotList::iterator eIt, eEnd;
            for (eIt = m_entitySnapshots.begin(), eEnd = m_entitySnapshots.end(); eIt != eEnd; ++eIt) {
                Model::EntitySnapshot& snapshot = *eIt;
                snapshot.restore();
            }
            
            BrushSnapshotList::iterator bIt, bEnd;
            for (bIt = m_brushSnapshots.begin(), bEnd = m_brushSnapshots.end(); bIt != bEnd; ++bIt) {
                Model::BrushSnapshot& snapshot = *bIt;
                snapshot.restore(worldBounds);
            }
            
            BrushFaceSnapshotList::iterator fIt, fEnd;
            for (fIt = m_faceSnapshots.begin(), fEnd = m_faceSnapshots.end(); fIt != fEnd; ++fIt) {
                Model::BrushFaceSnapshot& snapshot = *fIt;
                snapshot.restore();
            }
            
            m_entitySnapshots.clear();
            m_brushSnapshots.clear();
            m_faceSnapshots.clear();
        }
    }
}
