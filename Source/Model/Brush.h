/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "Model/BrushGeometry.h"
#include "Model/EditState.h"
#include "Model/FaceTypes.h"
#include "Model/MapObject.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Face;
        class Texture;
        
        class Brush : public MapObject {
        protected:
            Entity* m_entity;
            FaceList m_faces;
            BrushGeometry* m_geometry;
            
            EditState m_editState;
            unsigned int m_selectedFaceCount;
            
            const BBox& m_worldBounds;
            
            size_t m_filePosition;
            
            void init();
            void validateGeometry();
        public:
            Brush(const BBox& worldBounds);
            Brush(const BBox& worldBounds, const Brush& brushTemplate);
            Brush(const BBox& worldBounds, const BBox& brushBounds, Texture* texture);
            ~Brush();
            
            void restore(const Brush& brushTemplate, bool checkId = false);
            
            inline MapObject::Type objectType() const {
                return MapObject::Type::Brush;
            }
            
            inline Entity* entity() const {
                return m_entity;
            }
            
            void setEntity(Entity* entity);
            
            inline const FaceList& faces() const {
                return m_faces;
            }
            
            bool addFace(Face* face);
            
            inline bool selected() const {
                return m_editState == EditState::Selected;
            }
            
            inline bool partiallySelected() const {
                return m_selectedFaceCount > 0;
            }
            
            inline void incSelectedFaceCount() {
                m_selectedFaceCount++;
            }
            
            inline void decSelectedFaceCount() {
                m_selectedFaceCount--;
            }
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
            
            inline const Vec3f& center() const {
                return m_geometry->center;
            }
            
            inline const BBox& bounds() const {
                return m_geometry->bounds;
            }
            
            inline const VertexList& vertices() const {
                return m_geometry->vertices;
            }
            
            inline const EdgeList& edges() const {
                return m_geometry->edges;
            }
            
            inline bool closed() const {
                return m_geometry->closed();
            }

            inline size_t filePosition() const {
                return m_filePosition;
            }
            
            inline void setFilePosition(size_t filePosition) {
                m_filePosition = filePosition;
            }

            void pick(const Ray& ray, PickResult& pickResults, Filter& filter) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
