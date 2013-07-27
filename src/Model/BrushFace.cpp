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
#include "Assets/FaceTexture.h"
#include "Model/Brush.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        const Vec3 TextureCoordinateSystem::BaseAxes[] = {
            Vec3( 0.0,  0.0,  1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 0.0,  0.0, -1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3(-1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0,  1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0, -1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
        };
        
        TextureCoordinateSystem::TextureCoordinateSystem() :
        m_face(NULL),
        m_valid(false) {}
        
        void TextureCoordinateSystem::setFace(BrushFace* face) {
            assert(m_face == NULL);
            assert(face != NULL);
            m_face = face;
        }

        Vec2f TextureCoordinateSystem::textureCoordinates(const Vec3& vertex) const {
            if (!m_valid)
                validate();
            
            Assets::FaceTexture* texture = m_face->texture();
            const size_t width = texture != NULL ? texture->width() : 1;
            const size_t height = texture != NULL ? texture->height() : 1;
            
            const float x = static_cast<float>((vertex.dot(m_scaledTexAxisX) + m_face->xOffset()) / width);
            const float y = static_cast<float>((vertex.dot(m_scaledTexAxisY) + m_face->yOffset()) / height);
            return Vec2f(x, y);
        }
        
        void TextureCoordinateSystem::invalidate() {
            m_valid = false;
        }
        
        void TextureCoordinateSystem::validate() const {
            const Vec3& normal = m_face->boundary().normal;
            
            axesAndIndices(normal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            rotateAxes(m_texAxisX, m_texAxisY, Math<FloatType>::radians(m_face->rotation()), m_texPlaneNormIndex);
            m_scaledTexAxisX = m_texAxisX / (m_face->xScale() == 0.0f ? 1.0f : m_face->xScale());
            m_scaledTexAxisY = m_texAxisY / (m_face->yScale() == 0.0f ? 1.0f : m_face->yScale());
            
            m_valid = true;
        }
        
        void TextureCoordinateSystem::axesAndIndices(const Vec3& normal, Vec3& xAxis, Vec3& yAxis, size_t& planeNormIndex, size_t& faceNormIndex) const {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = BaseAxes[bestIndex * 3 + 1];
            yAxis = BaseAxes[bestIndex * 3 + 2];
            planeNormIndex = (bestIndex / 2) * 6;
            faceNormIndex = bestIndex * 3;
        }
        
        void TextureCoordinateSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex) const  {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(BaseAxes[planeNormIndex], planeNormIndex == 12 ? -angle : angle);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }
        
        const String BrushFace::NoTextureName = "__TB_empty";
        
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) :
        m_parent(NULL),
        m_textureName(textureName),
        m_xOffset(0.0f),
        m_yOffset(0.0f),
        m_rotation(0.0f),
        m_xScale(1.0f),
        m_yScale(1.0f),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_texture(NULL),
        m_side(NULL),
        m_vertexCacheValid(false) {
            m_textureCoordinateSystem.setFace(this);
            setPoints(point0, point1, point2);
        }
        
        Brush* BrushFace::parent() const {
            return m_parent;
        }
        
        void BrushFace::setParent(Brush* parent) {
            if (m_parent == parent)
                return;
            
            if (m_parent != NULL) {
                if (m_selected)
                    m_parent->decChildSelectionCount();
            }
            m_parent = parent;
            if (m_parent != NULL) {
                if (m_selected)
                    m_parent->incChildSelectionCount();
            }
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
        
        Assets::FaceTexture* BrushFace::texture() const {
            return m_texture;
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
        
        void BrushFace::setTexture(Assets::FaceTexture* texture) {
            if (m_texture != NULL)
                m_texture->decUsageCount();
            m_texture = texture;
            if (m_texture != NULL) {
                m_textureName = m_texture->name();
                m_texture->incUsageCount();
            }
            m_textureCoordinateSystem.invalidate();
        }

        void BrushFace::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setRotation(const float rotation) {
            m_rotation = rotation;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setXScale(const float xScale) {
            m_xScale = xScale;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setYScale(const float yScale) {
            m_yScale = yScale;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }
        
        void BrushFace::setSide(BrushFaceGeometry* side) {
            m_side = side;
        }
        
        bool BrushFace::selected() const {
            return m_selected;
        }
        
        void BrushFace::select() {
            if (m_selected)
                return;
            m_selected = true;
            if (m_parent != NULL)
                m_parent->incChildSelectionCount();
        }
        
        void BrushFace::deselect() {
            if (!m_selected)
                return;
            m_selected = false;
            if (m_parent != NULL)
                m_parent->decChildSelectionCount();
        }

        void BrushFace::addToMesh(Mesh& mesh) const {
            assert(m_side != NULL);
            if (!m_vertexCacheValid)
                validateVertexCache();
            
            mesh.beginTriangleSet(m_texture);
            mesh.addTrianglesToSet(m_cachedVertices);
            mesh.endTriangleSet();
        }

        FloatType BrushFace::intersectWithRay(const Ray3& ray) const {
            assert(m_side != NULL);
            
            const FloatType dot = m_boundary.normal.dot(ray.direction);
            if (!Math<FloatType>::neg(dot))
                return Math<FloatType>::nan();
            
            const FloatType dist = m_boundary.intersectWithRay(ray);
            if (Math<FloatType>::isnan(dist))
                return Math<FloatType>::nan();

            const size_t axis = m_boundary.normal.firstComponent();
            const Vec3 hit = ray.pointAtDistance(dist);
            const Vec3 projectedHit = swizzle(hit, axis);
            
            const BrushVertex::List& vertices = m_side->vertices();
            const BrushVertex* vertex = vertices.back();
            Vec3 v0 = swizzle(vertex->position(), axis) - projectedHit;
            
            int c = 0;
            for (size_t i = 0; i < vertices.size(); i++) {
                vertex = vertices[i];
                const Vec3 v1 = swizzle(vertex->position(), axis) - projectedHit;
                
                if ((Math<FloatType>::zero(v0.x()) && Math<FloatType>::zero(v0.y())) ||
                    (Math<FloatType>::zero(v1.x()) && Math<FloatType>::zero(v1.y()))) {
                    // the point is identical to a polygon vertex, cancel search
                    c = 1;
                    break;
                }
                
                /*
                 * A polygon edge intersects with the positive X axis if the
                 * following conditions are met: The Y coordinates of its
                 * vertices must have different signs (we assign a negative sign
                 * to 0 here in order to count it as a negative number) and one
                 * of the following two conditions must be met: Either the X
                 * coordinates of the vertices are both positive or the X
                 * coordinates of the edge have different signs (again, we
                 * assign a negative sign to 0 here). In the latter case, we
                 * must calculate the point of intersection between the edge and
                 * the X axis and determine whether its X coordinate is positive
                 * or zero.
                 */
                
                // do the Y coordinates have different signs?
                if ((v0.y() > 0.0 && v1.y() <= 0.0) || (v0.y() <= 0.0 && v1.y() > 0.0)) {
                    // Is segment entirely on the positive side of the X axis?
                    if (v0.x() > 0.0 && v1.x() > 0.0) {
                        c += 1; // edge intersects with the X axis
                        // if not, do the X coordinates have different signs?
                    } else if ((v0.x() > 0.0 && v1.x() <= 0.0) || (v0.x() <= 0.0 && v1.x() > 0.0)) {
                        // calculate the point of intersection between the edge
                        // and the X axis
                        const FloatType x = -v0.y() * (v1.x() - v0.x()) / (v1.y() - v0.y()) + v0.x();
                        if (x >= 0.0)
                            c += 1; // edge intersects with the X axis
                    }
                }
                
                v0 = v1;
            }
            
            if (c % 2 == 0)
                return Math<FloatType>::nan();
            return dist;
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

        void BrushFace::validateVertexCache() const {
            m_cachedVertices.clear();

            const BrushVertex::List& vertices = m_side->vertices();
            m_cachedVertices.reserve(3 * (vertices.size() - 2));
            
            for (size_t i = 1; i < vertices.size() - 1; i++) {
                m_cachedVertices.push_back(Vertex(vertices[0]->position(),
                                                  m_boundary.normal,
                                                  m_textureCoordinateSystem.textureCoordinates(vertices[0]->position())));
                m_cachedVertices.push_back(Vertex(vertices[i]->position(),
                                                  m_boundary.normal,
                                                  m_textureCoordinateSystem.textureCoordinates(vertices[i]->position())));
                m_cachedVertices.push_back(Vertex(vertices[i+1]->position(),
                                                  m_boundary.normal,
                                                  m_textureCoordinateSystem.textureCoordinates(vertices[i+1]->position())));
            }
            m_vertexCacheValid = true;
        }
    }
}
