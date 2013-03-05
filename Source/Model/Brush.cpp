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

#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/Filter.h"
#include "Model/Picker.h"
#include "Model/Texture.h"
#include "Utility/List.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        void Brush::init() {
            m_entity = NULL;
            m_filePosition = 0;
            setEditState(EditState::Default);
            m_selectedFaceCount = 0;
            m_geometry = new BrushGeometry(m_worldBounds);
        }

        Brush::Brush(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }
        
        Brush::Brush(const BBox& worldBounds, const Brush& brushTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            restore(brushTemplate, false);
        }
        
        Brush::Brush(const BBox& worldBounds, const BBox& brushBounds, Texture* texture) : MapObject(), m_worldBounds(worldBounds) {
            init();
            
            Vec3f p1, p2, p3;
            String textureName = texture != NULL ? texture->name() : "";
            
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
            setEntity(NULL);
            delete m_geometry;
            m_geometry = NULL;
            Utility::deleteAll(m_faces);
        }

        void Brush::restore(const Brush& brushTemplate, bool checkId) {
            if (checkId)
                assert(uniqueId() == brushTemplate.uniqueId());
            
            Utility::deleteAll(m_faces);
            delete m_geometry;
            m_geometry = new BrushGeometry(*brushTemplate.m_geometry);
            
            const FaceList templateFaces = brushTemplate.faces();
            for (size_t i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(m_worldBounds, *templateFaces[i]);
                face->setBrush(this);
                m_faces.push_back(face);
                
                for (size_t j = 0; j < m_geometry->sides.size(); j++) {
                    Side* side = m_geometry->sides[j];
                    if (side->face == templateFaces[i]) {
                        side->face = face;
                        face->setSide(side);
                        break;
                    }
                }
            }

            // snap(0);
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::setEntity(Entity* entity) {
            if (entity == m_entity)
                return;
            
            if (m_entity != NULL) {
                if (selected())
                    m_entity->decSelectedBrushCount();
                else if (hidden())
                    m_entity->decHiddenBrushCount();
            }
            
            m_entity = entity;
            
            if (m_entity != NULL) {
                if (selected())
                    m_entity->incSelectedBrushCount();
                else if (hidden())
                    m_entity->incHiddenBrushCount();
            }
        }

        bool Brush::addFace(Face* face) {
            try {
                FaceList droppedFaces;
                BrushGeometry::CutResult result = m_geometry->addFace(*face, droppedFaces);
                
                if (result == BrushGeometry::Redundant) {
                    delete face;
                    return true;
                }
                
                if (result == BrushGeometry::Null) {
                    delete face;
                    return false;
                }
                
                for (unsigned int i = 0; i < droppedFaces.size(); i++) {
                    Face* droppedFace = droppedFaces[i];
                    FaceList::iterator it = find(m_faces.begin(), m_faces.end(), droppedFace);
                    delete *it;
                    m_faces.erase(it);
                }
                
                face->setBrush(this);
                m_faces.push_back(face);

                if (m_entity != NULL)
                    m_entity->invalidateGeometry();
                
                return true;
            } catch (GeometryException e) {
                delete face;
                return false;
            }
        }
        
        void Brush::replaceFaces(const FaceList& newFaces) {
            Utility::deleteAll(m_faces);
            delete m_geometry;
            
            m_geometry = new BrushGeometry(m_worldBounds);
            for (unsigned int i = 0; i < newFaces.size(); i++)
                addFace(newFaces[i]);
        }
        
        EditState::Type Brush::setEditState(EditState::Type editState) {
            EditState::Type previous = MapObject::setEditState(editState);
            if (m_entity != NULL) {
                if (previous != EditState::Selected && editState == EditState::Selected)
                    m_entity->incSelectedBrushCount();
                else if (previous == EditState::Selected && editState != EditState::Selected)
                    m_entity->decSelectedBrushCount();
                if (previous != EditState::Hidden && editState == EditState::Hidden)
                    m_entity->incHiddenBrushCount();
                else if (previous == EditState::Hidden && editState!= EditState::Hidden)
                    m_entity->decHiddenBrushCount();
            }
            return previous;
        }

        void Brush::translate(const Vec3f& delta, bool lockTextures) {
            if (delta.null())
                return;
            
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.translate(delta, lockTextures);
            }

            m_geometry->translate(delta);
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTextures) {
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.rotate90(axis, center, clockwise, lockTextures);
            }

            m_geometry->rotate90(axis, center, clockwise);
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::rotate(const Quat& rotation, const Vec3f& center, bool lockTextures) {
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.rotate(rotation, center, lockTextures);
            }
            
            m_geometry->rotate(rotation, center);
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::flip(Axis::Type axis, const Vec3f& center, bool lockTextures) {
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.flip(axis, center, lockTextures);
            }
            
            m_geometry->flip(axis, center);
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::correct(float epsilon) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            m_geometry->correct(newFaces, droppedFaces, epsilon);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }
        
        void Brush::snap(unsigned int snapTo) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            m_geometry->snap(newFaces, droppedFaces, snapTo);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }
        
        bool Brush::canMoveBoundary(const Face& face, const Vec3f& delta) const {
            
            // using worldbounds here can lead to invalid brushes due to precision errors
            // so we use a smaller bounding box that's still big enough to fit the brush
            BBox maxBounds = m_geometry->bounds;
            float max = std::max(std::max(std::abs(delta.x), std::abs(delta.y)), std::abs(delta.z));
            maxBounds.expand(2.0f * max);

            BrushGeometry testGeometry(maxBounds);

            Face testFace(face);
            testFace.translate(delta, false);
            
            FaceList droppedFaces;
            FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Face* otherFace = *it;
                if (otherFace != &face)
                    testGeometry.addFace(*otherFace, droppedFaces);
            }
            
            BrushGeometry::CutResult result = testGeometry.addFace(testFace, droppedFaces);
            bool inWorldBounds = m_worldBounds.contains(testGeometry.bounds);
            
            m_geometry->restoreFaceSides();
            
            return inWorldBounds && result == BrushGeometry::Split && droppedFaces.empty();
        }
        
        void Brush::moveBoundary(Face& face, const Vec3f& delta, bool lockTexture) {
            assert(canMoveBoundary(face, delta));

            // using worldbounds here can lead to invalid brushes due to precision errors
            // so we use a smaller bounding box that's still big enough to fit the brush
            BBox maxBounds = m_geometry->bounds;
            float max = std::max(std::max(std::abs(delta.x), std::abs(delta.y)), std::abs(delta.z));
            maxBounds.expand(2.0f * max);
            
            face.translate(delta, lockTexture);
            delete m_geometry;
            m_geometry = new BrushGeometry(maxBounds);

            FaceList droppedFaces;
            FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Face& aFace = **it;
                BrushGeometry::CutResult result = m_geometry->addFace(aFace, droppedFaces);
                assert(result == BrushGeometry::Split);
                aFace.invalidateVertexCache();
            }
            
            assert(droppedFaces.empty());
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        bool Brush::canMoveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) const {
            return m_geometry->canMoveVertices(m_worldBounds, vertexPositions, delta);
        }

        Vec3f::List Brush::moveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;

            Vec3f::List newVertexPositions = m_geometry->moveVertices(m_worldBounds, vertexPositions, delta, newFaces, droppedFaces);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
            return newVertexPositions;
        }

        bool Brush::canMoveEdges(const EdgeList& edges, const Vec3f& delta) const {
            return m_geometry->canMoveEdges(m_worldBounds, edges, delta);
        }
        
        EdgeList Brush::moveEdges(const EdgeList& edges, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            const EdgeList result = m_geometry->moveEdges(m_worldBounds, edges, delta, newFaces, droppedFaces);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
            return result;
        }

        bool Brush::canMoveFaces(const FaceList& faces, const Vec3f& delta) const {
            return m_geometry->canMoveFaces(m_worldBounds, faces, delta);
        }
        
        FaceList Brush::moveFaces(const FaceList& faces, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            const FaceList result = m_geometry->moveFaces(m_worldBounds, faces, delta, newFaces, droppedFaces);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
            return result;
        }

        bool Brush::canSplitEdge(Edge* edge, const Vec3f& delta) const {
            return m_geometry->canSplitEdge(m_worldBounds, edge, delta);
        }
        
        Vec3f Brush::splitEdge(Edge* edge, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            Vec3f newVertexPosition = m_geometry->splitEdge(m_worldBounds, edge, delta, newFaces, droppedFaces);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
            return newVertexPosition;
        }
        
        bool Brush::canSplitFace(Face* face, const Vec3f& delta) const {
            return m_geometry->canSplitFace(m_worldBounds, face, delta);
        }

        Vec3f Brush::splitFace(Face* face, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;
            
            Vec3f newVertexPosition = m_geometry->splitFace(m_worldBounds, face, delta, newFaces, droppedFaces);
            
            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* dropFace = *it;
                dropFace->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), dropFace), m_faces.end());
                delete dropFace;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* keepFace = *it;
                face->invalidateTexAxes();
                keepFace->invalidateVertexCache();
            }
            
            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* newFace = *it;
                newFace->setBrush(this);
                m_faces.push_back(newFace);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
            return newVertexPosition;
        }

        void Brush::pick(const Ray& ray, PickResult& pickResults) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist))
                return;
            
            dist = Math::nan();
            Side* side;
            for (unsigned int i = 0; i < m_geometry->sides.size() && Math::isnan(dist); i++) {
                side = m_geometry->sides[i];
                dist = side->intersectWithRay(ray);
            }
            
            if (!Math::isnan(dist)) {
                Vec3f hitPoint = ray.pointAtDistance(dist);
                FaceHit* hit = new FaceHit(*side->face, hitPoint, dist);
                pickResults.add(hit);
            }
        }

        bool Brush::containsPoint(const Vec3f point) const {
            if (!bounds().contains(point))
                return false;
            
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& face = **faceIt;
                if (face.boundary().pointStatus(point) == PointStatus::PSAbove)
                    return false;
            }
            
            return true;
        }
        
        bool Brush::intersectsBrush(const Brush& brush) const {
            if (!bounds().intersects(brush.bounds()))
                return false;
            
            // separating axis theorem
            // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
            
            FaceList::const_iterator faceIt, faceEnd;

            const VertexList& myVertices = vertices();
            const FaceList& theirFaces = brush.faces();
            for (faceIt = theirFaces.begin(), faceEnd = theirFaces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& theirFace = **faceIt;
                const Vec3f& origin = theirFace.vertices().front()->position;
                const Vec3f& direction = theirFace.boundary().normal;
                if (vertexStatusFromRay(origin, direction, myVertices) == PointStatus::PSAbove)
                    return false;
            }

            const VertexList& theirVertices = brush.vertices();
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& myFace = **faceIt;
                const Vec3f& origin = myFace.vertices().front()->position;
                const Vec3f& direction = myFace.boundary().normal;
                if (vertexStatusFromRay(origin, direction, theirVertices) == PointStatus::PSAbove)
                    return false;
            }

            const EdgeList& myEdges = edges();
            const EdgeList& theirEdges = brush.edges();
            EdgeList::const_iterator myEdgeIt, myEdgeEnd, theirEdgeIt, theirEdgeEnd;
            for (myEdgeIt = myEdges.begin(), myEdgeEnd = myEdges.end(); myEdgeIt != myEdgeEnd; ++myEdgeIt) {
                const Edge& myEdge = **myEdgeIt;
                for (theirEdgeIt = theirEdges.begin(), theirEdgeEnd = theirEdges.end(); theirEdgeIt != theirEdgeEnd; ++theirEdgeIt) {
                    const Edge& theirEdge = **theirEdgeIt;
                    const Vec3f myEdgeVec = myEdge.vector();
                    const Vec3f theirEdgeVec = theirEdge.vector();
                    const Vec3f& origin = myEdge.start->position;
                    const Vec3f direction = myEdgeVec.crossed(theirEdgeVec);
                    
                    PointStatus::Type myStatus = vertexStatusFromRay(origin, direction, myVertices);
                    if (myStatus != PointStatus::PSInside) {
                        PointStatus::Type theirStatus = vertexStatusFromRay(origin, direction, theirVertices);
                        if (theirStatus != PointStatus::PSInside) {
                            if (myStatus != theirStatus)
                                return false;
                        }
                    }
                }
            }
            
            return true;
        }
        
        bool Brush::containsBrush(const Brush& brush) const {
            if (bounds().contains(brush.bounds()))
                return false;
            
            const VertexList& theirVertices = brush.vertices();
            VertexList::const_iterator vertexIt, vertexEnd;
            for (vertexIt = theirVertices.begin(), vertexEnd = theirVertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                const Vertex& vertex = **vertexIt;
                if (!containsPoint(vertex.position))
                    return false;
            }

            return true;
        }
        
        bool Brush::intersectsEntity(const Entity& entity) const {
            BBox theirBounds = entity.bounds();
            if (!bounds().intersects(theirBounds))
                return false;
            
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
        
        bool Brush::containsEntity(const Entity& entity) const {
            BBox theirBounds = entity.bounds();
            if (!bounds().contains(theirBounds))
                return false;
            
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
    }
}
