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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushFace.h"

#include "Exceptions.h"
#include "VecMath.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        const String BrushFace::NoTextureName = "__TB_empty";
        const BrushFace::List BrushFace::EmptyList = BrushFace::List();
        
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) :
        m_textureName(textureName),
        m_side(NULL) {
            setPoints(point0, point1, point2);
        }
        
        BrushFace::Ptr BrushFace::newBrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            return BrushFace::Ptr(new BrushFace(point0, point1, point2, textureName));
        }

        const BrushFace::Points& BrushFace::points() const {
            return m_points;
        }

        bool BrushFace::arePointsOnPlane(const Plane3& plane) const {
            for (size_t i = 0; i < 3; i++)
                if (plane.pointStatus(m_points[i]) != PointStatus::PSInside)
                    return false;
            return true;
        }

        const String& BrushFace::textureName() const {
            return m_textureName;
        }
        
        const Plane3& BrushFace::boundary() const {
            return m_boundary;
        }
        
        float BrushFace::xOffset() const {
            return m_xOffset;
        }
        
        float BrushFace::yOffset() const {
            return m_yOffset;
        }
        
        float BrushFace::rotation() const {
            return m_rotation;
        }
        
        float BrushFace::xScale() const {
            return m_xScale;
        }
        
        float BrushFace::yScale() const {
            return m_yScale;
        }
        
        void BrushFace::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
        }
        
        void BrushFace::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
        }
        
        void BrushFace::setRotation(const float rotation) {
            m_rotation = rotation;
        }
        
        void BrushFace::setXScale(const float xScale) {
            m_xScale = xScale;
        }
        
        void BrushFace::setYScale(const float yScale) {
            m_yScale = yScale;
        }
        
        void BrushFace::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }
        
        void BrushFace::setSide(BrushFaceGeometry* side) {
            m_side = side;
        }
        
        void BrushFace::addToMesh(Mesh& mesh) {
            assert(m_side != NULL);
            
            mesh.beginTriangleSet(m_textureName);
            
            const BrushVertex::List& vertices = m_side->vertices();
            for (size_t i = 1; i < vertices.size() - 1; i++) {
                const Renderer::VP3N3T2 v1(vertices[0]->position(),
                                           Vec3f(0.0f),
                                           Vec2f(0.0f));
                const Renderer::VP3N3T2 v2(vertices[i]->position(),
                                           Vec3f(0.0f),
                                           Vec2f(0.0f));
                const Renderer::VP3N3T2 v3(vertices[i+1]->position(),
                                           Vec3f(0.0f),
                                           Vec2f(0.0f));
                mesh.addTriangleToSet(v1, v2, v3);
            }
            
            mesh.endTriangleSet();
        }

        void BrushFace::setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            m_points[0] = point0;
            m_points[1] = point1;
            m_points[2] = point2;
            
            if (!setPlanePoints(m_boundary, m_points)) {
                GeometryException e;
                e << "Colinear face points: (" <<
                m_points[0].asString() << ") (" <<
                m_points[1].asString() << ") (" <<
                m_points[2].asString() << ")";
                throw e;
            }
        }
    }
}
