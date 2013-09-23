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
#include "Assets/FaceTexture.h"
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
        Clipper::ClipPoints::ClipPoints() :
        m_valid(false) {}
        
        Clipper::ClipPoints::ClipPoints(const Vec3& point1, const Vec3& point2, const Vec3& point3) :
        m_valid(true) {
            m_points[0] = point1;
            m_points[1] = point2;
            m_points[2] = point3;
        }

        void Clipper::ClipPoints::invert() {
            std::swap(m_points[1], m_points[2]);
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

        Clipper::Clipper(const Renderer::Camera& camera) :
        m_camera(camera),
        m_clipSide(Front),
        m_numPoints(0) {}

        bool Clipper::clipPointValid(const Vec3& point) const {
            if (m_numPoints == 3)
                return false;
            if (identicalWithAnyPoint(point, m_numPoints))
                return false;
            if (m_numPoints == 2 && linearlyDependent(m_points[0], m_points[1], point))
                return false;
            return true;
        }
        
        void Clipper::addClipPoint(const Vec3& point, const Model::BrushFace& face) {
            assert(m_numPoints < 3);
            assert(clipPointValid(point));
            m_points[m_numPoints] = point;
            m_normals[m_numPoints] = getNormals(point, face);
            ++m_numPoints;
        }

        void Clipper::deleteLastClipPoint() {
            assert(m_numPoints > 0);
            --m_numPoints;
        }

        size_t Clipper::indexOfPoint(const Vec3& point) const {
            for (size_t i = 0; i < m_numPoints; ++i)
                if (m_points[i] == point)
                    return i;
            return 3;
        }
        
        bool Clipper::pointUpdateValid(const size_t index, const Vec3& newPoint) {
            assert(index < m_numPoints);
            if (identicalWithAnyPoint(newPoint, index))
                return false;
            if (m_numPoints < 3)
                return true;
            
            switch (index) {
                case 0:
                    return !linearlyDependent(m_points[1], m_points[2], newPoint);
                case 1:
                    return !linearlyDependent(m_points[0], m_points[2], newPoint);
                case 2:
                    return !linearlyDependent(m_points[0], m_points[1], newPoint);
                default:
                    return false;
            }
        }

        void Clipper::updatePoint(const size_t index, const Vec3& point, const Model::BrushFace& face) {
            assert(index < m_numPoints);
            assert(pointUpdateValid(index, point));
            m_points[index] = point;
            m_normals[index] = getNormals(point, face);
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

        size_t Clipper::numPoints() const {
            return m_numPoints;
        }

        Vec3::List Clipper::clipPoints() const {
            Vec3::List result(m_numPoints);
            for (size_t i = 0; i < m_numPoints; ++i)
                result[i] = m_points[i];
            return result;
        }

        void Clipper::reset() {
            m_clipSide = Front;
            m_numPoints = 0;
        }

        ClipResult Clipper::clip(const Model::BrushList& brushes, const View::MapDocumentPtr document) const {
            ClipResult result;
            
            const ClipPoints points = computeClipPoints();
            if (points.valid()) {
                const BBox3& worldBounds = document->worldBounds();
                Model::Map& map = *document->map();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Entity* entity = brush->parent();
                    
                    Model::BrushFace* frontFace = map.createFace(points[0], points[1], points[2], document->currentTextureName());
                    Model::BrushFace* backFace = map.createFace(points[0], points[1], points[2], document->currentTextureName());
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
            }
            
            return result;
        }

        bool Clipper::identicalWithAnyPoint(const Vec3& point, const size_t disregardIndex) const {
            for (size_t i = 0; i < m_numPoints; ++i)
                if (i != disregardIndex && m_points[i] == point)
                    return true;
            return false;
        }

        bool Clipper::linearlyDependent(const Vec3& p1, const Vec3& p2, const Vec3& p3) const {
            const Vec3 v1 = (p3 - p1).normalized();
            const Vec3 v2 = (p3 - p2).normalized();
            const FloatType dot = v1.dot(v2);
            return Math::eq(std::abs(dot), 1.0);
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

        Clipper::ClipPoints Clipper::computeClipPoints() const {
            assert(m_numPoints <= 3);

            ClipPoints result;
            if (m_numPoints == 1) {
                assert(!m_normals[0].empty());
                if (m_normals[0].size() <= 2) { // the point is not on a vertex
                    const Vec3 normal = selectNormal(m_normals[0], Vec3::List());
                    Vec3 dir;
                    if (normal.firstComponent() == Math::Axis::AZ) {
                        if (m_camera.direction().firstComponent() != Math::Axis::AZ)
                            dir = m_camera.direction().firstAxis();
                        else
                            dir = m_camera.direction().secondAxis();
                    } else {
                        dir = normal.firstAxis();
                    }
                    result = ClipPoints(m_points[0].rounded(),
                                        m_points[0].rounded() + 128.0 * Vec3::PosZ,
                                        m_points[0].rounded() + 128.0 * dir);
                }
            } else if (m_numPoints == 2) {
                assert(!m_normals[0].empty());
                assert(!m_normals[1].empty());
                
                const Vec3 normal = selectNormal(m_normals[0], m_normals[1]);
                result = ClipPoints(m_points[0].rounded(),
                                    m_points[0].rounded() + 128.0 * normal.firstAxis(),
                                    m_points[2]);
            } else {
                result = ClipPoints(m_points[0].rounded(),
                                    m_points[1].rounded(),
                                    m_points[2].rounded());
            }
            
            // make sure the plane's normal points towards the camera or to its left if the camera position is on the plane
            if (result.valid()) {
                Plane3 plane;
                setPlanePoints(plane, result.points());
                if (plane.pointStatus(m_camera.position()) == Math::PointStatus::PSInside) {
                    if (plane.normal.dot(m_camera.right()) < 0.0)
                        result.invert();
                } else {
                    if (plane.normal.dot(m_camera.direction()) > 0.0)
                        result.invert();
                }
            }
            
            return result;
        }

        Vec3 Clipper::selectNormal(const Vec3::List& normals1, const Vec3::List& normals2) const {
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

        void Clipper::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace& frontFace, Model::BrushFace& backFace) const {
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
            }
            
            assert(bestFrontFace != NULL);
            assert(bestBackFace != NULL);
            frontFace.setAttributes(*bestFrontFace);
            backFace.setAttributes(*bestBackFace);
        }
    }
}
