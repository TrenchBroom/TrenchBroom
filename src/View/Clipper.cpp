/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Clipper.h"

#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Renderer/Camera.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        Clipper::ClipHandlePoints::ClipHandlePoints() :
        m_numPoints(0) {}
        
        size_t Clipper::ClipHandlePoints::numPoints() const {
            return m_numPoints;
        }
        
        size_t Clipper::ClipHandlePoints::indexOfPoint(const Vec3& position) const {
            for (size_t i = 0; i < m_numPoints; ++i)
                if (m_points[i].position == position)
                    return i;
            return 3;
        }

        const Clipper::ClipHandlePoint& Clipper::ClipHandlePoints::operator[](const size_t index) const {
            assert(index < m_numPoints);
            return m_points[index];
        }

        bool Clipper::ClipHandlePoints::canAddPoint(const Vec3& position) const {
            if (m_numPoints == 3)
                return false;
            if (identicalWithAnyPoint(position, m_numPoints))
                return false;
            if (m_numPoints == 2 && linearlyDependent(m_points[0].position, m_points[1].position, position))
                return false;
            return true;
        }
        
        void Clipper::ClipHandlePoints::addPoint(const Vec3& position, const Vec3::List& normals) {
            assert(m_numPoints < 3);
            assert(canAddPoint(position));
            m_points[m_numPoints].position = position;
            m_points[m_numPoints].normals = normals;
            ++m_numPoints;
        }
        
        bool Clipper::ClipHandlePoints::canUpdatePoint(const size_t index, const Vec3& position) {
            assert(index < m_numPoints);
            if (identicalWithAnyPoint(position, index))
                return false;
            if (m_numPoints < 3)
                return true;
            
            switch (index) {
                case 0:
                    return !linearlyDependent(m_points[1].position, m_points[2].position, position);
                case 1:
                    return !linearlyDependent(m_points[0].position, m_points[2].position, position);
                case 2:
                    return !linearlyDependent(m_points[0].position, m_points[1].position, position);
                default:
                    return false;
            }
        }
        
        void Clipper::ClipHandlePoints::updatePoint(const size_t index, const Vec3& position, const Vec3::List& normals) {
            assert(index < m_numPoints);
            assert(canUpdatePoint(index, position));
            m_points[index].position = position;
            m_points[index].normals = normals;
        }
        
        void Clipper::ClipHandlePoints::deleteLastPoint() {
            assert(m_numPoints > 0);
            --m_numPoints;
        }
        
        void Clipper::ClipHandlePoints::deleteAllPoints() {
            m_numPoints = 0;
        }
        
        bool Clipper::ClipHandlePoints::identicalWithAnyPoint(const Vec3& position, const size_t disregardIndex) const {
            for (size_t i = 0; i < m_numPoints; ++i)
                if (i != disregardIndex && m_points[i].position == position)
                    return true;
            return false;
        }
        
        bool Clipper::ClipHandlePoints::linearlyDependent(const Vec3& p1, const Vec3& p2, const Vec3& p3) const {
            const Vec3 v1 = (p3 - p1).normalized();
            const Vec3 v2 = (p3 - p2).normalized();
            const FloatType dot = v1.dot(v2);
            return Math::eq(std::abs(dot), 1.0);
        }
        
        Clipper::ClipPoints::ClipPoints(const ClipHandlePoints& handlePoints, const Vec3& viewDirection) :
        m_valid(handlePoints.numPoints() > 0) {
            const size_t numPoints = handlePoints.numPoints();
            if (numPoints == 1) {
                const ClipHandlePoint& point = handlePoints[0];
                assert(!point.normals.empty());
                
                if (point.normals.size() <= 2) { // the point is not on a vertex
                    const Vec3 normal = selectNormal(point.normals, Vec3::List());
                    Vec3 dir;
                    if (normal.firstComponent() == Math::Axis::AZ) {
                        if (viewDirection.firstComponent() != Math::Axis::AZ)
                            dir = viewDirection.firstAxis();
                        else
                            dir = viewDirection.secondAxis();
                    } else {
                        dir = normal.firstAxis();
                    }
                    m_points[0] = point.position.rounded();
                    m_points[1] = point.position.rounded() + 128.0 * Vec3::PosZ;
                    m_points[2] = point.position.rounded() + 128.0 * dir;
                }
            } else if (numPoints == 2) {
                const ClipHandlePoint& point0 = handlePoints[0];
                const ClipHandlePoint& point1 = handlePoints[1];
                
                assert(!point0.normals.empty());
                assert(!point1.normals.empty());
                
                const Vec3 normal = selectNormal(point0.normals, point1.normals);
                m_points[0] = point0.position.rounded();
                m_points[1] = point0.position.rounded() + 128.0 * normal.firstAxis();
                m_points[2] = point1.position.rounded();
            } else if (numPoints == 3) {
                for (size_t i = 0; i < 3; ++i)
                    m_points[i] = handlePoints[i].position.rounded();
            }
        }

        bool Clipper::ClipPoints::valid() const {
            return m_valid;
        }

        const Vec3& Clipper::ClipPoints::operator[](const size_t index) const {
            assert(index < 3);
            return m_points[index];
        }

        const Vec3* Clipper::ClipPoints::points() const {
            return m_points;
        }

        void Clipper::ClipPoints::invertPlaneNormal() {
            using std::swap;
            swap(m_points[1], m_points[2]);
        }
        
        Vec3 Clipper::ClipPoints::selectNormal(const Vec3::List& normals1, const Vec3::List& normals2) {
            assert(!normals1.empty());
            
            Vec3f sum;
            // first, try to find two normals with the same first axis
            for (size_t i = 0; i < normals1.size(); ++i) {
                const Vec3& normal1 = normals1[i];
                for (size_t j =  0; j < normals2.size(); ++j) {
                    const Vec3& normal2 = normals2[j];
                    if (normal1.firstAxis() == normal2.firstAxis())
                        return normal1;
                }
                sum += normal1;
            }
            
            for (size_t i = 0; i < normals2.size(); ++i)
                sum += normals2[i];
            
            return sum / static_cast<float>((normals1.size() + normals2.size()));
        }

        Clipper::Clipper(const Renderer::Camera& camera) :
        m_camera(camera),
        m_clipPoints(m_handlePoints, m_camera.direction()),
        m_clipSide(Front) {}

        size_t Clipper::numPoints() const {
            return m_handlePoints.numPoints();
        }
        
        Vec3::List Clipper::clipPointPositions() const {
            Vec3::List result(numPoints());
            for (size_t i = 0; i < numPoints(); ++i)
                result[i] = m_handlePoints[i].position;
            return result;
        }
        
        size_t Clipper::indexOfPoint(const Vec3& position) const {
            return m_handlePoints.indexOfPoint(position);
        }
        
        bool Clipper::keepFrontBrushes() const {
            return m_clipSide != Back;
        }
        
        bool Clipper::keepBackBrushes() const {
            return m_clipSide != Front;
        }
        
        bool Clipper::canAddClipPoint(const Vec3& position) const {
            return m_handlePoints.canAddPoint(position);
        }
        
        void Clipper::addClipPoint(const Vec3& position, const Model::BrushFace& face) {
            m_handlePoints.addPoint(position, getNormals(position, face));
            updateClipPoints();
            setClipPlaneNormal();
        }

        bool Clipper::canUpdateClipPoint(const size_t index, const Vec3& position) {
            return m_handlePoints.canUpdatePoint(index, position);
        }

        void Clipper::updateClipPoint(const size_t index, const Vec3& position, const Model::BrushFace& face) {
            m_handlePoints.updatePoint(index, position, getNormals(position, face));
            updateClipPoints();
        }

        void Clipper::deleteLastClipPoint() {
            m_handlePoints.deleteLastPoint();
            updateClipPoints();
            if (m_clipPoints.valid())
                setClipPlaneNormal();
        }
        
        void Clipper::reset() {
            m_handlePoints.deleteAllPoints();
            updateClipPoints();
        }
        
        void Clipper::toggleClipSide() {
            switch (m_clipSide) {
                case Front:
                    m_clipSide = Back;
                    break;
                case Back:
                    m_clipSide = Both;
                    break;
                default:
                    m_clipSide = Front;
                    break;
            }
        }

        ClipResult Clipper::clip(const Model::BrushList& brushes, const View::MapDocumentPtr document) const {
            ClipResult result;
            
            const BBox3& worldBounds = document->worldBounds();
            if (m_clipPoints.valid()) {
                Model::Map& map = *document->map();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Entity* entity = brush->parent();
                    
                    Model::BrushFace* frontFace = map.createFace(m_clipPoints[0], m_clipPoints[1], m_clipPoints[2], document->currentTextureName());
                    Model::BrushFace* backFace = map.createFace(m_clipPoints[0], m_clipPoints[2], m_clipPoints[1], document->currentTextureName());
                    setFaceAttributes(brush->faces(), *frontFace, *backFace);
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    if (frontBrush->clip(worldBounds, frontFace))
                        result.frontBrushes[entity].push_back(frontBrush);
                    else
                        delete frontBrush;
                    
                    Model::Brush* backBrush = brush->clone(worldBounds);
                    if (backBrush->clip(worldBounds, backFace))
                        result.backBrushes[entity].push_back(backBrush);
                    else
                        delete backBrush;
                }
            } else {
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Entity* entity = brush->parent();
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    result.frontBrushes[entity].push_back(frontBrush);
                }
            }
            
            return result;
        }

        Vec3::List Clipper::getNormals(const Vec3& point, const Model::BrushFace& face) const {
            const Model::Brush& brush = *face.parent();
            const Model::BrushEdgeList edges = brush.edges();
            Model::BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::BrushEdge& edge = **it;
                if (point.equals(edge.start()->position())) {
                    return getNormals(brush.incidentFaces(*edge.start()));
                } else if (point.equals(edge.end()->position())) {
                    return getNormals(brush.incidentFaces(*edge.end()));
                } else if (edge.contains(point)) {
                    Vec3::List normals(2);
                    normals[0] = edge.leftFace()->boundary().normal;
                    normals[1] = edge.rightFace()->boundary().normal;
                    return normals;
                }
            }
            
            Vec3::List normals(1);
            normals[0] = face.boundary().normal;
            return normals;
        }

        Vec3::List Clipper::getNormals(const Model::BrushFaceList& faces) const {
            Vec3::List normals;
            normals.reserve(faces.size());
        
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::BrushFace& face = **it;
                normals.push_back(face.boundary().normal);
            }
            
            return normals;
        }

        void Clipper::updateClipPoints() {
            m_clipPoints = ClipPoints(m_handlePoints, m_camera.direction());
        }
        
        void Clipper::setClipPlaneNormal() {
            assert(m_clipPoints.valid());
            
            // make sure the plane's normal points towards the camera or to its left if the camera position is on the plane
            Plane3 plane;
            setPlanePoints(plane, m_clipPoints[0], m_clipPoints[1], m_clipPoints[2]);
            if (plane.pointStatus(m_camera.position()) == Math::PointStatus::PSInside) {
                if (plane.normal.dot(m_camera.right()) < 0.0)
                    m_clipPoints.invertPlaneNormal();
            } else {
                if (plane.normal.dot(m_camera.direction()) > 0.0)
                    m_clipPoints.invertPlaneNormal();
            }
        }

        void Clipper::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace& frontFace, Model::BrushFace& backFace) const {
            assert(!faces.empty());
            
            Model::BrushFaceList::const_iterator faceIt = faces.begin();
            Model::BrushFaceList::const_iterator faceEnd = faces.end();
            const Model::BrushFace* bestFrontFace = *faceIt++;
            const Model::BrushFace* bestBackFace = bestFrontFace;
            
            while (faceIt != faceEnd) {
                const Model::BrushFace* face = *faceIt;
                
                const Vec3 bestFrontDiff = bestFrontFace->boundary().normal - frontFace.boundary().normal;
                const Vec3 frontDiff = face->boundary().normal - frontFace.boundary().normal;
                if (frontDiff.squaredLength() < bestFrontDiff.squaredLength())
                    bestFrontFace = face;
                
                const Vec3f bestBackDiff = bestBackFace->boundary().normal - backFace.boundary().normal;
                const Vec3f backDiff = face->boundary().normal - backFace.boundary().normal;
                if (backDiff.squaredLength() < bestBackDiff.squaredLength())
                    bestBackFace = face;
                ++faceIt;
            }
            
            assert(bestFrontFace != NULL);
            assert(bestBackFace != NULL);
            frontFace.setAttributes(*bestFrontFace);
            backFace.setAttributes(*bestBackFace);
        }
    }
}
