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

#ifndef TrenchBroom_Brush_h
#define TrenchBroom_Brush_h

#include <vector>
#include "Model/Assets/Texture.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Face.h"
#include "Model/Map/MapObject.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Face;
        class Vertex;
        class Edge;
        class BrushGeometry;
        class MoveResult;
        
        class Brush : public MapObject {
        protected:
            Entity* m_entity;
            vector<Face*> m_faces;
            
            BrushGeometry* m_geometry;
            
            bool m_onGrid;
            const BBox& m_worldBounds;
            
            int m_filePosition;
            bool m_selected;
            
            void init();
            void rebuildGeometry();
        public:
            Brush(const BBox& worldBounds);
            Brush(const BBox& worldBounds, const Brush& brushTemplate);
            Brush(const BBox& worldBounds, const BBox& brushBounds, Assets::Texture& texture);
            ~Brush();
            
            void restore(const Brush& brushTemplate);
            
            EMapObjectType objectType() const;
            Entity* entity() const;
            void setEntity(Entity* entity);
            const vector<Face*>& faces() const;
            const BBox& bounds() const;
            const BBox& worldBounds() const;
            const Vec3f center() const;
            const vector<Vertex*>& vertices() const;
            const vector<Edge*>& edges() const;
            
            void pick(const Ray& ray, HitList& hits);
            bool containsPoint(Vec3f point);
            bool intersectsBrush(Brush& brush);
            bool containsBrush(Brush& brush);
            bool intersectsEntity(Entity& entity);
            bool containsEntity(Entity& entity);
            
            bool addFace(Face* face);
            bool canDeleteFace(Face& face);
            void deleteFace(Face& face);
            
            void translate(const Vec3f& delta, bool lockTextures);
            void rotate90(EAxis axis, const Vec3f& center, bool clockwise, bool lockTextures);
            void rotate(const Quat& rotation, const Vec3f& center, bool lockTextures);
            void flip(EAxis axis, const Vec3f& center, bool lockTextures);
            bool canResize(Face& face, float dist);
            void resize(Face& face, float dist, bool lockTextures);
            void enlarge(float delta, bool lockTextures);
            void snap();
            
            MoveResult moveVertex(int vertexIndex, Vec3f delta);
            MoveResult moveEdge(int edgeIndex, Vec3f delta);
            MoveResult moveFace(int faceIndex, Vec3f delta);
            
            int filePosition() const;
            void setFilePosition(int filePosition);
            bool selected() const;
            void setSelected(bool selected);
        };
    }
}

#endif
