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

#include "Brush.h"
#include <assert.h>
#include <algorithm>
#include "Model/Assets/Texture.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Face.h"
#include "Model/Map/Picker.h"
#include "Utilities/Filter.h"

namespace TrenchBroom {
    namespace Model {
        void Brush::init() {
            entity = NULL;
            onGrid = false;
            filePosition = -1;
            selected = false;
            geometry = new BrushGeometry(worldBounds);
        }
        
        Brush::Brush(const BBox& worldBounds) : MapObject(), worldBounds(worldBounds) {
            init();
        }
        
        Brush::Brush(const BBox& worldBounds, const Brush& brushTemplate) : MapObject(), worldBounds(worldBounds) {
            init();
            restore(brushTemplate, false);
        }
        
        Brush::Brush(const BBox& worldBounds, const BBox& brushBounds, Assets::Texture* texture) : MapObject(), worldBounds(worldBounds) {
            init();
            
            Vec3f p1, p2, p3;
            std::string textureName = texture != NULL ? texture->name : "";
            
            p1 = brushBounds.min;
            p2 = p1;
            p2.z = brushBounds.max.z;
            p3 = p1;
            p3.x = brushBounds.max.x;
            Face* front = new Face(worldBounds, p1, p2, p3, textureName);
            front->setTexture(texture);
            addFace(front);
            
            p2 = p1;
            p2.y = brushBounds.max.y;
            p3 = p1;
            p3.z = brushBounds.max.z;
            Face* left = new Face(worldBounds, p1, p2, p3, textureName);
            left->setTexture(texture);
            addFace(left);
            
            p2 = p1;
            p2.x = brushBounds.max.x;
            p3 = p1;
            p3.y = brushBounds.max.y;
            Face* bottom = new Face(worldBounds, p1, p2, p3, textureName);
            bottom->setTexture(texture);
            addFace(bottom);
            
            p1 = brushBounds.max;
            p2 = p1;
            p2.x = brushBounds.min.x;
            p3 = p1;
            p3.z = brushBounds.min.z;
            Face* back = new Face(worldBounds, p1, p2, p3, textureName);
            back->setTexture(texture);
            addFace(back);
            
            p2 = p1;
            p2.z = brushBounds.min.z;
            p3 = p1;
            p3.y = brushBounds.min.y;
            Face* right = new Face(worldBounds, p1, p2, p3, textureName);
            right->setTexture(texture);
            addFace(right);
            
            p2 = p1;
            p2.y = brushBounds.min.y;
            p3 = p1;
            p3.x = brushBounds.min.x;
            Face* top = new Face(worldBounds, p1, p2, p3, textureName);
            top->setTexture(texture);
            addFace(top);
        }
        
        Brush::~Brush() {
            delete geometry;
			geometry = NULL;
            while(!faces.empty()) delete faces.back(), faces.pop_back();
        }
        
        void Brush::rebuildGeometry() {
            FaceList droppedFaces;
            
            delete geometry;
            geometry = new BrushGeometry(worldBounds);
            geometry->addFaces(faces, droppedFaces);
            for (unsigned int i = 0; i < droppedFaces.size(); i++) {
                Face* droppedFace = droppedFaces[i];
                FaceList::iterator it = find(faces.begin(), faces.end(), droppedFace);
                delete *it;
                faces.erase(it);
            }
        }
        
        void Brush::restore(const Brush& brushTemplate, bool checkId) {
            if (checkId)
                assert(uniqueId() == brushTemplate.uniqueId());
            
            while(!faces.empty()) delete faces.back(), faces.pop_back();
            if (geometry != NULL)
                delete geometry;
            geometry = new BrushGeometry(worldBounds);
            
            const std::vector<Face* > templateFaces = brushTemplate.faces;
            for (unsigned int i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(worldBounds, *templateFaces[i]);
                addFace(face);
            }
            
            if (entity != NULL)
                entity->brushChanged(this);
        }
        
        const BBox& Brush::bounds() const {
            return geometry->bounds;
        }

        EMapObjectType Brush::objectType() const {
            return TB_MT_BRUSH;
        }

        const Vec3f Brush::center() const {
            return centerOfVertices(geometry->vertices);
        }
        
        void Brush::pick(const Ray& ray, HitList& hits, Filter& filter) {
            if (!filter.brushPickable(*this))
                return;
            
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist))
                return;
            
            dist = std::numeric_limits<float>::quiet_NaN();
            Side* side;
            for (unsigned int i = 0; i < geometry->sides.size() && Math::isnan(dist); i++) {
                side = geometry->sides[i];
                dist = side->intersectWithRay(ray);
            }
            
            if (!Math::isnan(dist)) {
                Vec3f hitPoint = ray.pointAtDistance(dist);
                Hit* hit = new Hit(side->face, TB_HT_FACE, hitPoint, dist);
                hits.add(*hit);
            }
        }
        
        void Brush::pickVertices(const Ray& ray, float handleSize, HitList& hits) {
            const VertexList& vertices = geometry->vertices;
            for (unsigned int i = 0; i < vertices.size(); i++) {
                BBox handle(vertices[i]->position, handleSize);
                float dist = handle.intersectWithRay(ray);
                if (!Math::isnan(dist)) {
                    Vec3f hitPoint = ray.pointAtDistance(dist);
                    Hit* hit = new Hit(this, i, TB_HT_VERTEX_HANDLE, hitPoint, dist);
                    hits.add(*hit);
                }
            }
            
            const EdgeList& edges = geometry->edges;
            for (unsigned int i = 0; i < edges.size(); i++) {
                BBox handle(edges[i]->center(), handleSize);
                float dist = handle.intersectWithRay(ray);
                if (!Math::isnan(dist)) {
                    Vec3f hitPoint = ray.pointAtDistance(dist);
                    Hit* hit = new Hit(this, i, TB_HT_EDGE_HANDLE, hitPoint, dist);
                    hits.add(*hit);
                }
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                BBox handle(faces[i]->center(), handleSize);
                float dist = handle.intersectWithRay(ray);
                if (!Math::isnan(dist)) {
                    Vec3f hitPoint = ray.pointAtDistance(dist);
                    Hit* hit = new Hit(this, i, TB_HT_FACE_HANDLE, hitPoint, dist);
                    hits.add(*hit);
                }
            }
        }

        bool Brush::containsPoint(Vec3f point) {
            if (!bounds().contains(point)) return false;
            
            for (unsigned int i = 0; i < faces.size(); i++)
                if (faces[i]->boundary.pointStatus(point) == TB_PS_ABOVE)
                    return false;
            return true;
        }
        
        bool Brush::intersectsBrush(Brush& brush) {
            if (!bounds().intersects(brush.bounds())) return false;
            
            // separating axis theorem
            // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
            
            const VertexList& myVertices = geometry->vertices;
            const FaceList& theirFaces = brush.faces;
            for (unsigned int i = 0; i < theirFaces.size(); i++) {
                Face* theirFace = theirFaces[i];
                Vec3f origin = theirFace->side->vertices[0]->position;
                Vec3f direction = theirFace->boundary.normal;
                if (vertexStatusFromRay(origin, direction, myVertices) == TB_PS_ABOVE)
                    return false;
            }
            
            
            const VertexList& theirVertices = brush.geometry->vertices;
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* myFace = faces[i];
                Vec3f origin = myFace->side->vertices[0]->position;
                Vec3f direction = myFace->boundary.normal;
                if (vertexStatusFromRay(origin, direction, theirVertices) == TB_PS_ABOVE)
                    return false;
            }
            
            const EdgeList& myEdges = geometry->edges;
            const EdgeList& theirEdges = brush.geometry->edges;
            for (unsigned int i = 0; i < myEdges.size(); i++) {
                Edge* myEdge = myEdges[i];
                for (unsigned int j = 0; j < theirEdges.size(); j++) {
                    Edge* theirEdge = theirEdges[i];
                    Vec3f myEdgeVec = myEdge->end->position - myEdge->start->position;
                    Vec3f theirEdgeVec = theirEdge->end->position - theirEdge->start->position;
                    Vec3f direction = myEdgeVec % theirEdgeVec;
                    Vec3f origin = myEdge->start->position;
                    
                    EPointStatus myStatus = vertexStatusFromRay(origin, direction, myVertices);
                    if (myStatus != TB_PS_INSIDE) {
                        EPointStatus theirStatus = vertexStatusFromRay(origin, direction, theirVertices);
                        if (theirStatus != TB_PS_INSIDE) {
                            if (myStatus != theirStatus)
                                return false;
                        }
                    }
                }
            }
            
            return true;
        }
        
        bool Brush::containsBrush(Brush& brush) {
            if (bounds().contains(brush.bounds())) return false;
            
            const VertexList& theirVertices = brush.geometry->vertices;
            for (unsigned int i = 0; i < theirVertices.size(); i++)
                if (!containsPoint(theirVertices[i]->position))
                    return false;
            return true;
        }
        
        bool Brush::intersectsEntity(Entity& entity) {
            BBox theirBounds = entity.bounds();
            if (!bounds().intersects(theirBounds)) return false;
            
            Vec3f point = theirBounds.min;
            if (containsPoint(point))
                return true;
            point.x = theirBounds.max.x;
            if (containsPoint(point))
                return true;
            point.y = theirBounds.max.y;
            if (containsPoint(point))
                return true;
            point.x = theirBounds.min.x;
            if (containsPoint(point))
                return true;
            point = theirBounds.max;
            if (containsPoint(point))
                return true;
            point.x = theirBounds.min.x;
            if (containsPoint(point))
                return true;
            point.y = theirBounds.min.y;
            if (containsPoint(point))
                return true;
            point.x = theirBounds.max.x;
            if (containsPoint(point))
                return true;
            return false;
        }
        
        bool Brush::containsEntity(Entity& entity) {
            BBox theirBounds = entity.bounds();
            if (!bounds().contains(theirBounds)) return false;
            
            Vec3f point = theirBounds.min;
            if (!containsPoint(point))
                return false;
            point.x = theirBounds.max.x;
            if (!containsPoint(point))
                return false;
            point.y = theirBounds.max.y;
            if (!containsPoint(point))
                return false;
            point.x = theirBounds.min.x;
            if (!containsPoint(point))
                return false;
            point = theirBounds.max;
            if (!containsPoint(point))
                return false;
            point.x = theirBounds.min.x;
            if (!containsPoint(point))
                return false;
            point.y = theirBounds.min.y;
            if (!containsPoint(point))
                return false;
            point.x = theirBounds.max.x;
            if (!containsPoint(point))
                return false;
            return true;
        }
        
        bool Brush::addFace(Face* face) {
            try {
                FaceList droppedFaces;
                ECutResult result = geometry->addFace(*face, droppedFaces);
                
                if (result == TB_CR_REDUNDANT) {
                    delete face;
                    return true;
                }
                
                if (result == TB_CR_NULL) {
                    delete face;
                    return false;
                }
                
                for (unsigned int i = 0; i < droppedFaces.size(); i++) {
                    Face* droppedFace = droppedFaces[i];
                    FaceList::iterator it = find(faces.begin(), faces.end(), droppedFace);
                    delete *it;
                    faces.erase(it);
                }
                
                face->brush = this;
                faces.push_back(face);
                return true;
            } catch (GeometryException e) {
                delete face;
                return false;
            }
        }
        
        bool Brush::canDeleteFace(Face& face) {
            FaceList droppedFaces;
            BrushGeometry testGeometry(worldBounds);
            
            for (unsigned int i = 0; i < faces.size(); i++)
                if (faces[i] != &face)
                    testGeometry.addFace(*faces[i], droppedFaces);
            
            bool canDelete = testGeometry.closed();
            
            geometry->restoreFaceSides();
            return canDelete;
        }
        
        void Brush::deleteFace(Face& face) {
            FaceList::iterator it = find(faces.begin(), faces.end(), &face);
            delete *it;
            faces.erase(it);
            rebuildGeometry();
        }
        
        void Brush::replaceFaces(const FaceList& newFaces) {
            while (!faces.empty()) delete faces.back(), faces.pop_back();
            for (unsigned int i = 0; i < newFaces.size(); i++) {
                faces.push_back(newFaces[i]);
                newFaces[i]->brush = this;
            }
            rebuildGeometry();
        }

        void Brush::translate(const Vec3f& delta, bool lockTextures) {
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->translate(delta, lockTextures);
            geometry->translate(delta);
            entity->brushChanged(this);
        }
        
        void Brush::rotate90(EAxis axis, const Vec3f& center, bool clockwise, bool lockTextures) {
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->rotate90(axis, center, clockwise, lockTextures);
            geometry->rotate90(axis, center, clockwise);
            entity->brushChanged(this);
        }
        
        void Brush::rotate(const Quat& rotation, const Vec3f& center, bool lockTextures) {
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->rotate(rotation, center, lockTextures);
            geometry->rotate(rotation, center);
            entity->brushChanged(this);
        }
        
        void Brush::flip(EAxis axis, const Vec3f& center, bool lockTextures) {
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->flip(axis, center, lockTextures);
            geometry->flip(axis, center);
            entity->brushChanged(this);
        }
        
        bool Brush::canResize(Face& face, float dist) {
            Face testFace(worldBounds, face);
            testFace.move(dist, false);
            
            if (face.boundary.equals(testFace.boundary)) return false;
            
            FaceList droppedFaces;
            BrushGeometry testGeometry(worldBounds);
            for (unsigned int i = 0; i < faces.size(); i++)
                if (faces[i] != &face)
                    testGeometry.addFace(*faces[i], droppedFaces);
            
            ECutResult result = testGeometry.addFace(testFace, droppedFaces);
            bool canDrag = droppedFaces.size() == 0 && result == TB_CR_SPLIT && worldBounds.contains(testGeometry.bounds);
            
            geometry->restoreFaceSides();
            return canDrag;
        }
        
        void Brush::resize(Face& face, float dist, bool lockTextures) {
            face.move(dist, lockTextures);
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->coordsValid = false;
            rebuildGeometry();
            entity->brushChanged(this);
        }
        
        void Brush::enlarge(float delta, bool lockTextures) {
            for (unsigned int i = 0; i < faces.size(); i++)
                faces[i]->move(delta, lockTextures);
            rebuildGeometry();
            entity->brushChanged(this);
        }
        
        void Brush::snap() {
            geometry->snap();
            entity->brushChanged(this);
        }
        
        MoveResult Brush::moveVertex(size_t vertexIndex, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            FaceList::iterator faceIt;
            FaceList::iterator newFaceIt;
            FaceList::iterator droppedFaceIt;
            
            MoveResult result = geometry->moveVertex(vertexIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = droppedFaces.begin(); droppedFaceIt != droppedFaces.end(); ++droppedFaceIt) {
                Model::Face* face = *droppedFaceIt;
                face->brush = NULL;
                faceIt = find(faces.begin(), faces.end(), face);
                assert(faceIt != faces.end());
                delete face;
                faces.erase(faceIt);
            }
            
            for (faceIt = faces.begin(); faceIt != faces.end(); ++faceIt)
                (*faceIt)->coordsValid = false;
            
            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); ++newFaceIt) {
                Model::Face* face = *newFaceIt;
                face->brush = this;
                faces.push_back(face);
            }
            
            entity->brushChanged(this);
            return result;
        }
        
        MoveResult Brush::moveEdge(size_t edgeIndex, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            FaceList::iterator faceIt;
            FaceList::iterator newFaceIt;
            FaceList::iterator droppedFaceIt;
            
            MoveResult result = geometry->moveEdge(edgeIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = droppedFaces.begin(); droppedFaceIt != droppedFaces.end(); ++droppedFaceIt) {
                Model::Face* face = *droppedFaceIt;
                face->brush = NULL;
                faceIt = find(faces.begin(), faces.end(), face);
                assert(faceIt != faces.end());
                delete face;
                faces.erase(faceIt);
            }
            
            for (faceIt = faces.begin(); faceIt != faces.end(); ++faceIt)
                (*faceIt)->coordsValid = false;

            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); ++newFaceIt) {
                Model::Face* face = *newFaceIt;
                face->brush = this;
                faces.push_back(face);
            }
            
            entity->brushChanged(this);
            return result;
        }
        
        MoveResult Brush::moveFace(size_t faceIndex, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            FaceList::iterator faceIt;
            FaceList::iterator newFaceIt;
            FaceList::iterator droppedFaceIt;
            
            MoveResult result = geometry->moveSide(faceIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = droppedFaces.begin(); droppedFaceIt != droppedFaces.end(); ++droppedFaceIt) {
                Model::Face* face = *droppedFaceIt;
                face->brush = NULL;
                faceIt = find(faces.begin(), faces.end(), face);
                assert(faceIt != faces.end());
                delete face;
                faces.erase(faceIt);
            }
            
            for (faceIt = faces.begin(); faceIt != faces.end(); ++faceIt)
                (*faceIt)->coordsValid = false;

            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); ++newFaceIt) {
                Model::Face* face = *newFaceIt;
                face->brush = this;
                faces.push_back(face);
            }
            
            entity->brushChanged(this);
            return result;
        }
    }
}