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
            while (!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
        }

        void Brush::restore(const Brush& brushTemplate, bool checkId) {
            if (checkId)
                assert(uniqueId() == brushTemplate.uniqueId());
            
            while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
            delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);
            
            const FaceList templateFaces = brushTemplate.faces();
            for (unsigned int i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(m_worldBounds, *templateFaces[i]);
                addFace(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::setEntity(Entity* entity) {
            if (entity == m_entity)
                return;
            
            if (m_entity != NULL && selected())
                m_entity->decSelectedBrushCount();
            
            m_entity = entity;
            if (m_entity != NULL && selected())
                m_entity->incSelectedBrushCount();
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
            while (!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
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

        bool Brush::canMoveBoundary(const Face& face, const Vec3f& delta) {
            FaceList droppedFaces;
            BrushGeometry testGeometry(m_worldBounds);

            Face testFace(face);
            testFace.translate(delta, false);
            
            FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Face* otherFace = *it;
                if (otherFace != &face)
                    testGeometry.addFace(*otherFace, droppedFaces);
            }
            
            BrushGeometry::CutResult result = testGeometry.addFace(testFace, droppedFaces);
            m_geometry->restoreFaceSides();
            
            return result == BrushGeometry::Split;
        }
        
        void Brush::moveBoundary(Face& face, const Vec3f& delta, bool lockTexture) {
            assert(canMoveBoundary(face, delta));

            face.translate(delta, lockTexture);
            delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);

            FaceList droppedFaces;
            FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Face& aFace = **it;
                BrushGeometry::CutResult result = m_geometry->addFace(aFace, droppedFaces);
                assert(result == BrushGeometry::Split);
                aFace.invalidateVertexCache();
            }
            
            assert(droppedFaces.empty());
            m_entity->invalidateGeometry();
        }

        bool Brush::canMoveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) {
            return m_geometry->canMoveVertices(vertexPositions, delta);
        }

        Vec3f::List Brush::moveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;

            Vec3f::List newVertexPositions = m_geometry->moveVertices(vertexPositions, delta, newFaces, droppedFaces);
            
            for (FaceList::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateVertexCache();
            }
            
            for (FaceList::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            m_entity->invalidateGeometry();
            return newVertexPositions;
        }

        bool Brush::canMoveEdges(const EdgeList& edges, const Vec3f& delta) {
            return m_geometry->canMoveEdges(edges, delta);
        }
        
        void Brush::moveEdges(const EdgeList& edges, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            
            m_geometry->moveEdges(edges, delta, newFaces, droppedFaces);
            
            for (FaceList::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateVertexCache();
            }
            
            for (FaceList::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            m_entity->invalidateGeometry();
        }

        bool Brush::canMoveFaces(const FaceList& faces, const Vec3f& delta) {
            return m_geometry->canMoveFaces(faces, delta);
        }
        
        void Brush::moveFaces(const FaceList& faces, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            
            m_geometry->moveFaces(faces, delta, newFaces, droppedFaces);
            
            for (FaceList::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateVertexCache();
            }
            
            for (FaceList::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            m_entity->invalidateGeometry();
        }

        bool Brush::canSplitEdge(Edge* edge, const Vec3f& delta) {
            return m_geometry->canSplitEdge(edge, delta);
        }
        
        Vec3f Brush::splitEdge(Edge* edge, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            
            Vec3f newVertexPosition = m_geometry->splitEdge(edge, delta, newFaces, droppedFaces);
            
            for (FaceList::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), face), m_faces.end());
                delete face;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateVertexCache();
            }
            
            for (FaceList::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }
            
            m_entity->invalidateGeometry();
            return newVertexPosition;
        }
        
        bool Brush::canSplitFace(Face* face, const Vec3f& delta) {
            return m_geometry->canSplitFace(face, delta);
        }

        Vec3f Brush::splitFace(Face* face, const Vec3f& delta) {
            FaceList newFaces;
            FaceList droppedFaces;
            
            Vec3f newVertexPosition = m_geometry->splitFace(face, delta, newFaces, droppedFaces);
            
            for (FaceList::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* dropFace = *it;
                dropFace->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), dropFace), m_faces.end());
                delete dropFace;
            }
            
            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* keepFace = *it;
                keepFace->invalidateVertexCache();
            }
            
            for (FaceList::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* newFace = *it;
                newFace->setBrush(this);
                m_faces.push_back(newFace);
            }
            
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
    }
}
