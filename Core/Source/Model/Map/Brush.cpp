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
#include "Model/Map/Picker.h"

namespace TrenchBroom {
    namespace Model {
        void Brush::init() {
            m_entity = NULL;
            m_onGrid = false;
            m_filePosition = -1;
            m_selected = false;
            m_geometry = new BrushGeometry(m_worldBounds);
        }
        
        Brush::Brush(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }
        
        Brush::Brush(const BBox& worldBounds, const Brush& brushTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            restore(brushTemplate);
        }
        
        Brush::Brush(const BBox& worldBounds, const BBox& brushBounds, Assets::Texture& texture) : MapObject(), m_worldBounds(worldBounds) {
            init();
            
            Vec3f p1, p2, p3;
            
            p1 = brushBounds.min;
            p2 = p1;
            p2.z = brushBounds.max.z;
            p3 = p1;
            p3.x = brushBounds.max.x;
            Face* front = new Face(m_worldBounds, p1, p2, p3);
            front->setTexture(&texture);
            addFace(front);
            
            p2 = p1;
            p2.y = brushBounds.max.y;
            p3 = p1;
            p3.z = brushBounds.max.z;
            Face* left = new Face(m_worldBounds, p1, p2, p3);
            left->setTexture(&texture);
            addFace(left);
            
            p2 = p1;
            p2.x = brushBounds.max.x;
            p3 = p1;
            p3.y = brushBounds.max.y;
            Face* bottom = new Face(m_worldBounds, p1, p2, p3);
            bottom->setTexture(&texture);
            addFace(bottom);
            
            p1 = brushBounds.max;
            p2 = p1;
            p2.x = brushBounds.min.x;
            p3 = p1;
            p3.z = brushBounds.min.z;
            Face* back = new Face(m_worldBounds, p1, p2, p3);
            back->setTexture(&texture);
            addFace(back);
            
            p2 = p1;
            p2.z = brushBounds.min.z;
            p3 = p1;
            p3.y = brushBounds.min.y;
            Face* right = new Face(m_worldBounds, p1, p2, p3);
            right->setTexture(&texture);
            addFace(right);
            
            p2 = p1;
            p2.y = brushBounds.min.y;
            p3 = p1;
            p3.x = brushBounds.min.x;
            Face* top = new Face(m_worldBounds, p1, p2, p3);
            top->setTexture(&texture);
            addFace(top);
        }
        
        Brush::~Brush() {
            delete m_geometry;
            while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
        }
        
        void Brush::rebuildGeometry() {
            vector<Face*> droppedFaces;
            
            delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);
            m_geometry->addFaces(m_faces, droppedFaces);
            for (int i = 0; i < droppedFaces.size(); i++) {
                Face* droppedFace = droppedFaces[i];
                vector<Face*>::iterator it = find(m_faces.begin(), m_faces.end(), droppedFace);
                delete *it;
                m_faces.erase(it);
            }
        }
        
        void Brush::restore(const Brush& brushTemplate) {
            assert(uniqueId() == brushTemplate.uniqueId());
            
            while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
            if (m_geometry != NULL)
                delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);
            
            vector<Face* > templateFaces = brushTemplate.faces();
            for (int i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(m_worldBounds, *templateFaces[i]);
                addFace(face);
            }
            m_entity->brushChanged(this);
        }
        
        EMapObjectType Brush::objectType() const {
            return TB_MT_BRUSH;
        }

        Entity* Brush::entity() const {
            return m_entity;
        }
        
        void Brush::setEntity(Entity* entity) {
            m_entity = entity;
        }
        
        const vector<Face*>& Brush::faces() const {
            return m_faces;
        }
        
        const BBox& Brush::bounds() const {
            return m_geometry->bounds;
        }
        
        const BBox& Brush::worldBounds() const {
            return m_worldBounds;
        }
        
        const Vec3f Brush::center() const {
            return centerOfVertices(vertices());
        }
        
        const vector<Vertex*>& Brush::vertices() const {
            return m_geometry->vertices;
        }
        
        const vector<Edge*>& Brush::edges() const {
            return m_geometry->edges;
        }
        
        void Brush::pick(const Ray& ray, HitList& hits) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist)) return;
            
            dist = numeric_limits<float>::quiet_NaN();
            Side* side;
            for (int i = 0; i < m_geometry->sides.size() && Math::isnan(dist); i++) {
                side = m_geometry->sides[i];
                dist = side->intersectWithRay(ray);
            }
            
            if (!Math::isnan(dist)) {
                Vec3f hitPoint = ray.pointAtDistance(dist);
                Hit* hit = new Hit(side->face, TB_HT_FACE, hitPoint, dist);
                hits.add(*hit);
            }
        }
        
        bool Brush::containsPoint(Vec3f point) {
            if (!bounds().contains(point)) return false;
            
            for (int i = 0; i < m_faces.size(); i++)
                if (m_faces[i]->boundary().pointStatus(point) == TB_PS_ABOVE)
                    return false;
            return true;
        }
        
        bool Brush::intersectsBrush(Brush& brush) {
            if (!bounds().intersects(brush.bounds())) return false;
            
            // separating axis theorem
            // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
            
            const vector<Vertex*>& myVertices = vertices();
            const vector<Face*>& theirFaces = brush.faces();
            for (int i = 0; i < theirFaces.size(); i++) {
                Face* theirFace = theirFaces[i];
                Vec3f origin = theirFace->vertices()[0]->position;
                Vec3f direction = theirFace->boundary().normal;
                if (vertexStatusFromRay(origin, direction, myVertices) == TB_PS_ABOVE)
                    return false;
            }
            
            
            const vector<Vertex*>& theirVertices = brush.vertices();
            for (int i = 0; i < m_faces.size(); i++) {
                Face* myFace = m_faces[i];
                Vec3f origin = myFace->vertices()[0]->position;
                Vec3f direction = myFace->boundary().normal;
                if (vertexStatusFromRay(origin, direction, theirVertices) == TB_PS_ABOVE)
                    return false;
            }
            
            const vector<Edge*>& myEdges = edges();
            const vector<Edge*>& theirEdges = brush.edges();
            for (int i = 0; i < myEdges.size(); i++) {
                Edge* myEdge = myEdges[i];
                for (int j = 0; j < theirEdges.size(); j++) {
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
            
            const vector<Vertex*>& theirVertices = brush.vertices();
            for (int i = 0; i < theirVertices.size(); i++)
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
            return true;
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
            vector<Face*> droppedFaces;
            ECutResult result = m_geometry->addFace(*face, droppedFaces);
            if (result == TB_CR_REDUNDANT) return true;
            if (result == TB_CR_NULL) return false;
            
            for (int i = 0; i < droppedFaces.size(); i++) {
                Face* droppedFace = droppedFaces[i];
                vector<Face*>::iterator it = find(m_faces.begin(), m_faces.end(), droppedFace);
                delete *it;
                m_faces.erase(it);
            }
            face->setBrush(this);
            m_faces.push_back(face);
            return true;
        }
        
        bool Brush::canDeleteFace(Face& face) {
            vector<Face*> droppedFaces;
            BrushGeometry testGeometry(m_worldBounds);
            
            for (int i = 0; i < m_faces.size(); i++)
                if (m_faces[i] != &face)
                    testGeometry.addFace(*m_faces[i], droppedFaces);
            
            bool canDelete = testGeometry.closed();
            
            m_geometry->restoreFaceSides();
            return canDelete;
        }
        
        void Brush::deleteFace(Face& face) {
            vector<Face*>::iterator it = find(m_faces.begin(), m_faces.end(), &face);
            delete *it;
            m_faces.erase(it);
            rebuildGeometry();
        }
        
        void Brush::translate(const Vec3f& delta, bool lockTextures) {
            for (int i = 0; i < m_faces.size(); i++)
                m_faces[i]->translate(delta, lockTextures);
            m_geometry->translate(delta);
            m_entity->brushChanged(this);
        }
        
        void Brush::rotate90(EAxis axis, const Vec3f& center, bool clockwise, bool lockTextures) {
            for (int i = 0; i < m_faces.size(); i++)
                m_faces[i]->rotate90(axis, center, clockwise, lockTextures);
            m_geometry->rotate90(axis, center, clockwise);
            m_entity->brushChanged(this);
        }
        
        void Brush::rotate(const Quat& rotation, const Vec3f& center, bool lockTextures) {
            for (int i = 0; i < m_faces.size(); i++)
                m_faces[i]->rotate(rotation, center, lockTextures);
            m_geometry->rotate(rotation, center);
            m_entity->brushChanged(this);
        }
        
        void Brush::flip(EAxis axis, const Vec3f& center, bool lockTextures) {
            for (int i = 0; i < m_faces.size(); i++)
                m_faces[i]->flip(axis, center, lockTextures);
            m_geometry->flip(axis, center);
            m_entity->brushChanged(this);
        }
        
        bool Brush::canResize(Face& face, float dist) {
            Face testFace(m_worldBounds, face);
            testFace.move(dist, false);
            
            if (face.boundary().equals(testFace.boundary())) return false;
            
            vector<Face*> droppedFaces;
            BrushGeometry testGeometry(m_worldBounds);
            for (int i = 0; i < m_faces.size(); i++)
                if (m_faces[i] != &face)
                    testGeometry.addFace(*m_faces[i], droppedFaces);
            
            ECutResult result = testGeometry.addFace(testFace, droppedFaces);
            bool canDrag = droppedFaces.size() == 0 && result != TB_CR_NULL && m_worldBounds.contains(testGeometry.bounds);
            
            m_geometry->restoreFaceSides();
            return canDrag;
        }
        
        void Brush::resize(Face& face, float dist, bool lockTextures) {
            face.move(dist, lockTextures);
            rebuildGeometry();
            m_entity->brushChanged(this);
        }
        
        void Brush::enlarge(float delta, bool lockTextures) {
            for (int i = 0; i < m_faces.size(); i++)
                m_faces[i]->move(delta, lockTextures);
            rebuildGeometry();
            m_entity->brushChanged(this);
        }
        
        void Brush::snap() {
            m_geometry->snap();
            m_entity->brushChanged(this);
        }
        
        MoveResult Brush::moveVertex(int vertexIndex, Vec3f delta) {
            vector<Face*> newFaces;
            vector<Face*> droppedFaces;
            vector<Face*>::iterator faceIt;
            vector<Face*>::iterator newFaceIt;
            vector<Face*>::iterator droppedFaceIt;
            
            MoveResult result = m_geometry->moveVertex(vertexIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = newFaces.begin(); droppedFaceIt != newFaces.end(); droppedFaceIt++) {
                (*droppedFaceIt)->setBrush(NULL);
                faceIt = find(m_faces.begin(), m_faces.end(), *droppedFaceIt);
                delete *faceIt;
                m_faces.erase(faceIt);
            }
            
            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); newFaceIt++) {
                (*newFaceIt)->setBrush(this);
                m_faces.push_back(*newFaceIt);
            }
            
            m_entity->brushChanged(this);
            return result;
        }
        
        MoveResult Brush::moveEdge(int edgeIndex, Vec3f delta) {
            vector<Face*> newFaces;
            vector<Face*> droppedFaces;
            vector<Face*>::iterator faceIt;
            vector<Face*>::iterator newFaceIt;
            vector<Face*>::iterator droppedFaceIt;
            
            MoveResult result = m_geometry->moveEdge(edgeIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = newFaces.begin(); droppedFaceIt != newFaces.end(); droppedFaceIt++) {
                (*droppedFaceIt)->setBrush(NULL);
                faceIt = find(m_faces.begin(), m_faces.end(), *droppedFaceIt);
                delete *faceIt;
                m_faces.erase(faceIt);
            }
            
            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); newFaceIt++) {
                (*newFaceIt)->setBrush(this);
                m_faces.push_back(*newFaceIt);
            }
            
            m_entity->brushChanged(this);
            return result;
        }
        
        MoveResult Brush::moveFace(int faceIndex, Vec3f delta) {
            vector<Face*> newFaces;
            vector<Face*> droppedFaces;
            vector<Face*>::iterator faceIt;
            vector<Face*>::iterator newFaceIt;
            vector<Face*>::iterator droppedFaceIt;
            
            MoveResult result = m_geometry->moveSide(faceIndex, delta, newFaces, droppedFaces);
            
            for (droppedFaceIt = newFaces.begin(); droppedFaceIt != newFaces.end(); droppedFaceIt++) {
                (*droppedFaceIt)->setBrush(NULL);
                faceIt = find(m_faces.begin(), m_faces.end(), *droppedFaceIt);
                delete *faceIt;
                m_faces.erase(faceIt);
            }
            
            for (newFaceIt = newFaces.begin(); newFaceIt != newFaces.end(); newFaceIt++) {
                (*newFaceIt)->setBrush(this);
                m_faces.push_back(*newFaceIt);
            }
            
            m_entity->brushChanged(this);
            return result;
        }
        
        int Brush::filePosition() const {
            return m_filePosition;
        }
        
        void Brush::setFilePosition(int filePosition) {
            m_filePosition = filePosition;
        }
        
        bool Brush::selected() const {
            return m_selected;
        }
        
        void Brush::setSelected(bool selected) {
            m_selected = selected;
        }
    }
}