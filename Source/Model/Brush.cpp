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
            setEditState(EditState::Default);
            m_selectedFaceCount = 0;
            m_needsRebuild = false;
        }

        Brush::Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const FaceList& faces) :
        MapObject(),
        m_geometry(NULL),
        m_worldBounds(worldBounds),
        m_forceIntegerFacePoints(forceIntegerFacePoints) {
            init();

            FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }

            rebuildGeometry();
        }

        Brush::Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Brush& brushTemplate) :
        MapObject(),
        m_geometry(NULL),
        m_worldBounds(worldBounds),
        m_forceIntegerFacePoints(forceIntegerFacePoints) {
            init();
            restore(brushTemplate, false);
        }

        Brush::Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const BBoxf& brushBounds, Texture* texture) :
        MapObject(),
        m_geometry(NULL),
        m_worldBounds(worldBounds),
        m_forceIntegerFacePoints(forceIntegerFacePoints) {
            init();

            Vec3f p1, p2, p3;
            String textureName = texture != NULL ? texture->name() : "";

            p1 = brushBounds.min;
            p2 = p1;
            p2[2] = brushBounds.max[2];
            p3 = p1;
            p3[0] = brushBounds.max[0];
            Face* front = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            front->setTexture(texture);
            front->setBrush(this);
            m_faces.push_back(front);

            p2 = p1;
            p2[1] = brushBounds.max[1];
            p3 = p1;
            p3[2] = brushBounds.max[2];
            Face* left = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            left->setTexture(texture);
            left->setBrush(this);
            m_faces.push_back(left);

            p2 = p1;
            p2[0] = brushBounds.max[0];
            p3 = p1;
            p3[1] = brushBounds.max[1];
            Face* bottom = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            bottom->setTexture(texture);
            bottom->setBrush(this);
            m_faces.push_back(bottom);

            p1 = brushBounds.max;
            p2 = p1;
            p2[0] = brushBounds.min[0];
            p3 = p1;
            p3[2] = brushBounds.min[2];
            Face* back = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            back->setTexture(texture);
            back->setBrush(this);
            m_faces.push_back(back);

            p2 = p1;
            p2[2] = brushBounds.min[2];
            p3 = p1;
            p3[1] = brushBounds.min[1];
            Face* right = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            right->setTexture(texture);
            right->setBrush(this);
            m_faces.push_back(right);

            p2 = p1;
            p2[1] = brushBounds.min[1];
            p3 = p1;
            p3[0] = brushBounds.min[0];
            Face* top = new Face(worldBounds, m_forceIntegerFacePoints, p1, p2, p3, textureName);
            top->setTexture(texture);
            top->setBrush(this);
            m_faces.push_back(top);

            rebuildGeometry();
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

            const FaceList templateFaces = brushTemplate.faces();
            for (size_t i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(m_worldBounds, m_forceIntegerFacePoints, *templateFaces[i]);
                face->setBrush(this);
                m_faces.push_back(face);
            }

            rebuildGeometry();
        }

        void Brush::restore(const FaceList& faces) {
            Utility::deleteAll(m_faces);

            FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }

            rebuildGeometry();
        }

        void Brush::setEntity(Entity* entity) {
            if (entity == m_entity)
                return;

            if (m_entity != NULL) {
                if (selected())
                    m_entity->decSelectedBrushCount();
                else if (hidden())
                    m_entity->decHiddenBrushCount();
                if (entity == NULL && m_geometry != NULL) {
                    delete m_geometry;
                    m_geometry = NULL;
                }
            } else if (entity != NULL && m_geometry == NULL) {
                rebuildGeometry();
            }

            m_entity = entity;

            if (m_entity != NULL) {
                if (selected())
                    m_entity->incSelectedBrushCount();
                else if (hidden())
                    m_entity->incHiddenBrushCount();
            }
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

        void Brush::setForceIntegerFacePoints(bool forceIntegerFacePoints) {
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.setForceIntegerFacePoints(forceIntegerFacePoints);
            }

            m_forceIntegerFacePoints = forceIntegerFacePoints;
            rebuildGeometry();
        }

        void Brush::rebuildGeometry() {
            delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);

            // sort the faces by the weight of their plane normals like QBSP does
            Model::FaceList sortedFaces = m_faces;
            std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Planef::WeightOrder(true)));
            std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Planef::WeightOrder(false)));

            FaceSet droppedFaces;
            bool success = m_geometry->addFaces(sortedFaces, droppedFaces);
            assert(success);

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

            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTextures, const bool invertOrientation) {
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.transform(pointTransform, vectorTransform, lockTextures, invertOrientation);
            }

            rebuildGeometry();
        }

        bool Brush::clip(Face& face) {
            try {
                face.setBrush(this);
                m_faces.push_back(&face);
                rebuildGeometry();
                return !m_faces.empty() && closed();
            } catch (GeometryException&) {
                return false;
            }
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

            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }

            rebuildGeometry();
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

            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* face = *it;
                face->setBrush(this);
                m_faces.push_back(face);
            }

            rebuildGeometry();
        }

        bool Brush::canMoveBoundary(const Face& face, const Vec3f& delta) const {

            const Mat4f pointTransform = translationMatrix(delta);
            BrushGeometry testGeometry(m_worldBounds);

            Face testFace(face);
            testFace.transform(pointTransform, Mat4f::Identity, false, false);

            FaceSet droppedFaces;
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

            const Mat4f pointTransform = translationMatrix(delta);
            face.transform(pointTransform, Mat4f::Identity, false, false);
            rebuildGeometry();
        }

        bool Brush::canMoveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) const {
            return m_geometry->canMoveVertices(m_worldBounds, vertexPositions, delta);
        }

        Vec3f::List Brush::moveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;

            const Vec3f::List newVertexPositions = m_geometry->moveVertices(m_worldBounds, vertexPositions, delta, newFaces, droppedFaces);

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

            setNeedsRebuild(true);
            
            return newVertexPositions;
        }

        bool Brush::canMoveEdges(const EdgeInfoList& edgeInfos, const Vec3f& delta) const {
            return m_geometry->canMoveEdges(m_worldBounds, edgeInfos, delta);
        }

        EdgeInfoList Brush::moveEdges(const EdgeInfoList& edgeInfos, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;

            const EdgeInfoList newEdgeInfos = m_geometry->moveEdges(m_worldBounds, edgeInfos, delta, newFaces, droppedFaces);

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

            setNeedsRebuild(true);
            
            return newEdgeInfos;
        }

        bool Brush::canMoveFaces(const FaceInfoList& faceInfos, const Vec3f& delta) const {
            return m_geometry->canMoveFaces(m_worldBounds, faceInfos, delta);
        }

        FaceInfoList Brush::moveFaces(const FaceInfoList& faceInfos, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;

            const FaceInfoList newFaceInfos = m_geometry->moveFaces(m_worldBounds, faceInfos, delta, newFaces, droppedFaces);

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
            
            setNeedsRebuild(true);

            return newFaceInfos;
        }

        bool Brush::canSplitEdge(const EdgeInfo& edge, const Vec3f& delta) const {
            return m_geometry->canSplitEdge(m_worldBounds, edge, delta);
        }

        Vec3f Brush::splitEdge(const EdgeInfo& edge, const Vec3f& delta) {
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
            
            setNeedsRebuild(true);

            return newVertexPosition;
        }

        bool Brush::canSplitFace(const FaceInfo& faceInfo, const Vec3f& delta) const {
            return m_geometry->canSplitFace(m_worldBounds, faceInfo, delta);
        }

        Vec3f Brush::splitFace(const FaceInfo& faceInfo, const Vec3f& delta) {
            FaceSet newFaces;
            FaceSet droppedFaces;

            Vec3f newVertexPosition = m_geometry->splitFace(m_worldBounds, faceInfo, delta, newFaces, droppedFaces);

            for (FaceSet::iterator it = droppedFaces.begin(); it != droppedFaces.end(); ++it) {
                Face* dropFace = *it;
                dropFace->setBrush(NULL);
                m_faces.erase(std::remove(m_faces.begin(), m_faces.end(), dropFace), m_faces.end());
                delete dropFace;
            }

            for (FaceList::iterator it = m_faces.begin(); it != m_faces.end(); ++it) {
                Face* face = *it;
                face->invalidateTexAxes();
                face->invalidateVertexCache();
            }

            for (FaceSet::iterator it = newFaces.begin(); it != newFaces.end(); ++it) {
                Face* newFace = *it;
                newFace->setBrush(this);
                m_faces.push_back(newFace);
            }

            setNeedsRebuild(true);
            
            return newVertexPosition;
        }

        void Brush::pick(const Rayf& ray, PickResult& pickResults) {
            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math<float>::isnan(dist))
                return;

            dist = Math<float>::nan();
            Side* side = NULL;
            for (unsigned int i = 0; i < m_geometry->sides.size() && Math<float>::isnan(dist); i++) {
                side = m_geometry->sides[i];
                dist = side->intersectWithRay(ray);
            }

            if (!Math<float>::isnan(dist)) {
                assert(side != NULL);
                Vec3f hitPoint = ray.pointAtDistance(dist);
                FaceHit* hit = new FaceHit(*(side->face), hitPoint, dist);
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
                    const Vec3f direction = crossed(myEdgeVec, theirEdgeVec);

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
            BBoxf theirBounds = entity.bounds();
            if (!bounds().intersects(theirBounds))
                return false;

            Vec3f point = theirBounds.min;
            if (containsPoint(point))
                return true;
            point[0] = theirBounds.max[0];
            if (containsPoint(point))
                return true;
            point[1] = theirBounds.max[1];
            if (containsPoint(point))
                return true;
            point[0] = theirBounds.min[0];
            if (containsPoint(point))
                return true;
            point = theirBounds.max;
            if (containsPoint(point))
                return true;
            point[0] = theirBounds.min[0];
            if (containsPoint(point))
                return true;
            point[1] = theirBounds.min[1];
            if (containsPoint(point))
                return true;
            point[0] = theirBounds.max[0];
            if (containsPoint(point))
                return true;
            return false;
        }

        bool Brush::containsEntity(const Entity& entity) const {
            BBoxf theirBounds = entity.bounds();
            if (!bounds().contains(theirBounds))
                return false;

            Vec3f point = theirBounds.min;
            if (!containsPoint(point))
                return false;
            point[0] = theirBounds.max[0];
            if (!containsPoint(point))
                return false;
            point[1] = theirBounds.max[1];
            if (!containsPoint(point))
                return false;
            point[0] = theirBounds.min[0];
            if (!containsPoint(point))
                return false;
            point = theirBounds.max;
            if (!containsPoint(point))
                return false;
            point[0] = theirBounds.min[0];
            if (!containsPoint(point))
                return false;
            point[1] = theirBounds.min[1];
            if (!containsPoint(point))
                return false;
            point[0] = theirBounds.max[0];
            if (!containsPoint(point))
                return false;
            return true;
        }
    }
}
