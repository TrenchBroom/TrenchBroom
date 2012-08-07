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
#include "Model/Map/FaceTypes.h"
#include "Model/Map/MapExceptions.h"
#include "Model/Map/MapObject.h"

namespace TrenchBroom {
    class Filter;
    
    namespace Model {
        namespace Assets {
            class Texture;
        }
        
        class Entity;
        class Face;
        class Vertex;
        class Edge;
        class BrushGeometry;
        class MoveResult;
        
        class Brush : public MapObject {
        protected:
            void init();
            void rebuildGeometry();
        public:
            Entity* entity;
            FaceList faces;
            
            BrushGeometry* geometry;
            
            bool onGrid;
            const BBox& worldBounds;
            
            int filePosition;
            bool selected;
            bool partiallySelected;

            Brush(const BBox& worldBounds);
            Brush(const BBox& worldBounds, const Brush& brushTemplate);
            Brush(const BBox& worldBounds, const BBox& brushBounds, Assets::Texture* texture);
            ~Brush();
            
            void restore(const Brush& brushTemplate, bool checkId = false);
            
            const BBox& bounds() const;
            EMapObjectType objectType() const;
            const Vec3f center() const;
            
            void pick(const Ray& ray, HitList& hits, Filter& filter);
            void pickVertices(const Ray& ray, float handleSize, HitList& hits);
            void pickClosestFace(const Ray& ray, float maxDistance, HitList& hits);
            bool containsPoint(Vec3f point);
            bool intersectsBrush(Brush& brush);
            bool containsBrush(Brush& brush);
            bool intersectsEntity(Entity& entity);
            bool containsEntity(Entity& entity);
            
            bool addFace(Face* face);
            bool canDeleteFace(Face& face);
            void deleteFace(Face& face);
            void replaceFaces(const FaceList& newFaces);
            
            void translate(const Vec3f& delta, bool lockTextures);
            void rotate90(EAxis axis, const Vec3f& center, bool clockwise, bool lockTextures);
            void rotate(const Quat& rotation, const Vec3f& center, bool lockTextures);
            void flip(EAxis axis, const Vec3f& center, bool lockTextures);
            bool canResize(Face& face, float dist);
            void resize(Face& face, float dist, bool lockTextures);
            void enlarge(float delta, bool lockTextures);
            void snap();
            
            MoveResult moveVertex(size_t vertexIndex, const Vec3f& delta);
            MoveResult moveEdge(size_t edgeIndex, const Vec3f& delta);
            MoveResult moveFace(size_t faceIndex, const Vec3f& delta);
        };
    }
}

#endif
