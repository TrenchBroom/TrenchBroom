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

namespace TrenchBroom {
    namespace Model {
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            setPoints(point0, point1, point2);
        }
        
        const BrushFacePoints& BrushFace::points() const {
            return m_points;
        }
        
        const Plane3& BrushFace::boundary() const {
            return m_boundary;
        }

        const float BrushFace::xOffset() const {
            return m_xOffset;
        }
        
        const float BrushFace::yOffset() const {
            return m_yOffset;
        }
        
        const float BrushFace::rotation() const {
            return m_rotation;
        }
        
        const float BrushFace::xScale() const {
            return m_xScale;
        }
        
        const float BrushFace::yScale() const {
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

        void BrushFace::setEdges(const BrushEdgeList& edges) {
            m_edges = edges;
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
