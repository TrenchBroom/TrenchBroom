/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "BrushFace.h"

#include "Exceptions.h"
#include "VecMath.h"
#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/PlanePointFinder.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        BrushFaceAttribs::BrushFaceAttribs(const String& textureName) :
        m_textureName(textureName),
        m_texture(NULL),
        m_offset(Vec2f::Null),
        m_scale(Vec2f(1.0f, 1.0f)),
        m_rotation(0.0f),
        m_surfaceContents(0),
        m_surfaceFlags(0),
        m_surfaceValue(0.0f) {}
        
        const String& BrushFaceAttribs::textureName() const {
            return m_textureName;
        }
        
        Assets::Texture* BrushFaceAttribs::texture() const {
            return m_texture;
        }
        
        Vec2f BrushFaceAttribs::textureSize() const {
            if (m_texture == NULL)
                return Vec2f::One;
            const float w = m_texture->width()  == 0 ? 1.0f : static_cast<float>(m_texture->width());
            const float h = m_texture->height() == 0 ? 1.0f : static_cast<float>(m_texture->height());
            return Vec2f(w, h);
        }

        const Vec2f& BrushFaceAttribs::offset() const {
            return m_offset;
        }
        
        float BrushFaceAttribs::xOffset() const {
            return m_offset.x();
        }
        
        float BrushFaceAttribs::yOffset() const {
            return m_offset.y();
        }

        const Vec2f& BrushFaceAttribs::scale() const {
            return m_scale;
        }
        
        float BrushFaceAttribs::xScale() const {
            return m_scale.x();
        }
        
        float BrushFaceAttribs::yScale() const {
            return m_scale.y();
        }

        float BrushFaceAttribs::rotation() const {
            return m_rotation;
        }

        int BrushFaceAttribs::surfaceContents() const {
            return m_surfaceContents;
        }
        
        int BrushFaceAttribs::surfaceFlags() const {
            return m_surfaceFlags;
        }
        
        float BrushFaceAttribs::surfaceValue() const {
            return m_surfaceValue;
        }
        
        void BrushFaceAttribs::setTexture(Assets::Texture* texture) {
            m_texture = texture;
            if (m_texture != NULL)
                m_textureName = texture->name();
            else
                m_textureName = BrushFace::NoTextureName;
        }
        
        void BrushFaceAttribs::setOffset(const Vec2f& offset) {
            m_offset = offset;
        }

        void BrushFaceAttribs::setXOffset(const float xOffset) {
            m_offset[0] = xOffset;
        }
        
        void BrushFaceAttribs::setYOffset(const float yOffset) {
            m_offset[1] = yOffset;
        }

        void BrushFaceAttribs::setScale(const Vec2f& scale) {
            m_scale = scale;
        }
        
        void BrushFaceAttribs::setXScale(const float xScale) {
            m_scale[0] = xScale;
        }
        
        void BrushFaceAttribs::setYScale(const float yScale) {
            m_scale[1] = yScale;
        }

        void BrushFaceAttribs::setRotation(const float rotation) {
            m_rotation = rotation;
        }
        
        void BrushFaceAttribs::setSurfaceContents(const int surfaceContents) {
            m_surfaceContents = surfaceContents;
        }
        
        void BrushFaceAttribs::setSurfaceFlags(const int surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
        }
        
        void BrushFaceAttribs::setSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
        }

        BrushFaceSnapshot::BrushFaceSnapshot(BrushFace& face) :
        m_face(&face),
        m_attribs(m_face->attribs()) {}
        
        void BrushFaceSnapshot::restore() {
            m_face->setAttribs(m_attribs);
        }

        const String BrushFace::NoTextureName = "__TB_empty";
        
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName, TexCoordSystem* texCoordSystem) :
        m_parent(NULL),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_texCoordSystem(texCoordSystem),
        m_side(NULL),
        m_vertexCacheValid(false),
        m_attribs(textureName) {
            assert(m_texCoordSystem != NULL);
            setPoints(point0, point1, point2);
        }

        BrushFace* BrushFace::createParaxial(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            return new BrushFace(point0, point1, point2, textureName, new ParaxialTexCoordSystem(point0, point1, point2));
        }
        
        BrushFace* BrushFace::createParallel(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            return new BrushFace(point0, point1, point2, textureName, new ParallelTexCoordSystem(point0, point1, point2));
        }

        BrushFace::~BrushFace() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i] = Vec3::Null;
            m_parent = NULL;
            m_lineNumber = 0;
            m_lineCount = 0;
            m_selected = false;
            delete m_texCoordSystem;
            m_texCoordSystem = NULL;
            m_side = NULL;
            m_cachedVertices.clear();
            m_vertexCacheValid = false;
        }
        
        BrushFace* BrushFace::clone() const {
            BrushFace* result = new BrushFace(points()[0], points()[1], points()[2], textureName(), m_texCoordSystem->clone());
            result->m_attribs = m_attribs;
            result->setFilePosition(m_lineNumber, m_lineCount);
            if (m_selected)
                result->select();
            return result;
        }

        BrushFaceSnapshot BrushFace::takeSnapshot() {
            return BrushFaceSnapshot(*this);
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
                if (plane.pointStatus(m_points[i]) != Math::PointStatus::PSInside)
                    return false;
            return true;
        }
        
        const Plane3& BrushFace::boundary() const {
            return m_boundary;
        }
        
        Vec3 BrushFace::center() const {
            assert(m_side != NULL);
            return centerOfVertices(m_side->vertices);
        }

        Vec3 BrushFace::boundsCenter() const {
            assert(m_side != NULL);
            
            const Mat4x4 toPlane = planeProjectionMatrix(m_boundary.distance, m_boundary.normal);
            const Mat4x4 fromPlane = invertedMatrix(toPlane);
            
            BBox3 bounds;
            bounds.min = bounds.max = toPlane * m_side->vertices[0]->position;
            for (size_t i = 1; i < m_side->vertices.size(); ++i)
                bounds.mergeWith(toPlane * m_side->vertices[i]->position);
            return fromPlane * bounds.center();
        }

        const BrushFaceAttribs& BrushFace::attribs() const {
            return m_attribs;
        }
        
        void BrushFace::setAttribs(const BrushFaceAttribs& attribs) {
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->decUsageCount();
            
            const float oldRotation = m_attribs.rotation();
            m_attribs = attribs;
            
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->incUsageCount();

            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, m_attribs.rotation());
            invalidate();
        }

        const String& BrushFace::textureName() const {
            return m_attribs.textureName();
        }
        
        Assets::Texture* BrushFace::texture() const {
            return m_attribs.texture();
        }
        
        const Vec2f& BrushFace::offset() const {
            return m_attribs.offset();
        }
        
        float BrushFace::xOffset() const {
            return m_attribs.xOffset();
        }
        
        float BrushFace::yOffset() const {
            return m_attribs.yOffset();
        }
        
        const Vec2f& BrushFace::scale() const {
            return m_attribs.scale();
        }
        
        float BrushFace::xScale() const {
            return m_attribs.xScale();
        }
        
        float BrushFace::yScale() const {
            return m_attribs.yScale();
        }
        
        float BrushFace::rotation() const {
            return m_attribs.rotation();
        }
        
        int BrushFace::surfaceContents() const {
            return m_attribs.surfaceContents();
        }
        
        int BrushFace::surfaceFlags() const {
            return m_attribs.surfaceFlags();
        }
        
        float BrushFace::surfaceValue() const {
            return m_attribs.surfaceValue();
        }

        void BrushFace::setTexture(Assets::Texture* texture) {
            if (texture == m_attribs.texture())
                return;
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->decUsageCount();
            m_attribs.setTexture(texture);
            invalidateVertexCache();
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->incUsageCount();
        }
        
        void BrushFace::setXOffset(const float i_xOffset) {
            if (i_xOffset == xOffset())
                return;
            
            m_attribs.setXOffset(i_xOffset);
            invalidateVertexCache();
        }
        
        void BrushFace::setYOffset(const float i_yOffset) {
            if (i_yOffset == yOffset())
                return;
            m_attribs.setYOffset(i_yOffset);
            invalidateVertexCache();
        }
        
        void BrushFace::setXScale(const float i_xScale) {
            if (i_xScale == xScale())
                return;
            m_attribs.setXScale(i_xScale);
            invalidateVertexCache();
        }
        
        void BrushFace::setYScale(const float i_yScale) {
            if (i_yScale == yScale())
                return;
            m_attribs.setYScale(i_yScale);
            invalidateVertexCache();
        }
        
        void BrushFace::setRotation(const float rotation) {
            if (rotation == m_attribs.rotation())
                return;

            const float oldRotation = m_attribs.rotation();
            m_attribs.setRotation(rotation);
            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, rotation);
            invalidateVertexCache();
        }

        void BrushFace::setSurfaceContents(const int surfaceContents) {
            if (surfaceContents == m_attribs.surfaceContents())
                return;
            m_attribs.setSurfaceContents(surfaceContents);
        }
        
        void BrushFace::setSurfaceFlags(const int surfaceFlags) {
            if (surfaceFlags == m_attribs.surfaceFlags())
                return;
            m_attribs.setSurfaceFlags(surfaceFlags);
        }
        
        void BrushFace::setSurfaceValue(const float surfaceValue) {
            if (surfaceValue == m_attribs.surfaceValue())
                return;
            m_attribs.setSurfaceValue(surfaceValue);
        }

        void BrushFace::setAttributes(const BrushFace& other) {
            setTexture(other.texture());
            setXOffset(other.xOffset());
            setYOffset(other.yOffset());
            setRotation(other.rotation());
            setXScale(other.xScale());
            setYScale(other.yScale());
            setSurfaceContents(other.surfaceContents());
            setSurfaceFlags(other.surfaceFlags());
            setSurfaceValue(other.surfaceValue());
        }

        Vec3 BrushFace::textureXAxis() const {
            return m_texCoordSystem->xAxis();
        }
        
        Vec3 BrushFace::textureYAxis() const {
            return m_texCoordSystem->yAxis();
        }

        void BrushFace::moveTexture(const Vec3& up, const Vec3& right, const Vec2f& offset) {
            m_texCoordSystem->moveTexture(m_boundary.normal, up, right, offset, m_attribs);
            invalidateVertexCache();
        }

        void BrushFace::rotateTexture(const float angle) {
            m_texCoordSystem->rotateTexture(m_boundary.normal, angle, m_attribs);
            invalidate();
        }

        void BrushFace::transform(const Mat4x4& transform, const bool lockTexture) {
            using std::swap;

            m_texCoordSystem->transform(m_boundary.normal, transform, m_attribs, lockTexture);
            
            m_boundary.transform(transform);
            for (size_t i = 0; i < 3; ++i)
                m_points[i] = transform * m_points[i];
            if (crossed(m_points[2] - m_points[0], m_points[1] - m_points[0]).dot(m_boundary.normal) < 0.0)
                swap(m_points[1], m_points[2]);
            correctPoints();
            invalidateVertexCache();
        }

        void BrushFace::invert() {
            using std::swap;

            m_boundary.flip();
            swap(m_points[1], m_points[2]);
            invalidateVertexCache();
        }

        void BrushFace::updatePointsFromVertices() {
            Vec3 v1, v2;
            
            assert(m_side != NULL);
            const size_t vertexCount = m_side->vertices.size();
            assert(vertexCount >= 3);
            
            // Find a triple of consecutive vertices s.t. the (normalized) vectors from the mid vertex to the other two
            // have the smallest dot value of all such triples. This is to have better precision when computing the
            // boundary plane normal from these vectors.
            FloatType bestDot = 1.0;
            size_t best = vertexCount;
            for (size_t i = 0; i < vertexCount && bestDot > 0; ++i) {
                m_points[2] = m_side->vertices[Math::pred(i, vertexCount)]->position;
                m_points[0] = m_side->vertices[i]->position;
                m_points[1] = m_side->vertices[Math::succ(i, vertexCount)]->position;
                
                v1 = (m_points[2] - m_points[0]).normalized();
                v2 = (m_points[1] - m_points[0]).normalized();
                const FloatType dot = std::abs(v1.dot(v2));
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            setPoints(m_side->vertices[best]->position,
                      m_side->vertices[Math::succ(best, vertexCount)]->position,
                      m_side->vertices[Math::pred(best, vertexCount)]->position);
            invalidateVertexCache();
        }

        void BrushFace::snapPlanePointsToInteger() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i].round();
            setPoints(m_points[0], m_points[1], m_points[2]);
            invalidateVertexCache();
        }
        
        void BrushFace::findIntegerPlanePoints() {
            PlanePointFinder::findPoints(m_boundary, m_points, 3);
            setPoints(m_points[0], m_points[1], m_points[2]);
            invalidateVertexCache();
        }

        Mat4x4 BrushFace::projectToBoundaryMatrix() const {
            const Vec3 texZAxis = m_texCoordSystem->fromMatrix(Vec2f::Null, Vec2f::One) * Vec3::PosZ;
            const Mat4x4 worldToPlaneMatrix = planeProjectionMatrix(m_boundary.distance, m_boundary.normal, texZAxis);
            const Mat4x4 planeToWorldMatrix = invertedMatrix(worldToPlaneMatrix);
            return planeToWorldMatrix * Mat4x4::ZerZ * worldToPlaneMatrix;
        }

        Mat4x4 BrushFace::toTexCoordSystemMatrix(const Vec2f& offset, const Vec2f& scale, const bool project) const {
            if (project)
                return Mat4x4::ZerZ * m_texCoordSystem->toMatrix(offset, scale);
            else
                return m_texCoordSystem->toMatrix(offset, scale);
        }
        
        Mat4x4 BrushFace::fromTexCoordSystemMatrix(const Vec2f& offset, const Vec2f& scale, const bool project) const {
            if (project)
                return projectToBoundaryMatrix() * m_texCoordSystem->fromMatrix(offset, scale);
            return m_texCoordSystem->fromMatrix(offset, scale);
        }
        
        float BrushFace::measureTextureAngle(const Vec2f& center, const Vec2f& point) const {
            return m_texCoordSystem->measureAngle(m_attribs.rotation(), center, point);
        }

        const BrushEdgeList& BrushFace::edges() const {
            assert(m_side != NULL);
            return m_side->edges;
        }

        const BrushVertexList& BrushFace::vertices() const {
            assert(m_side != NULL);
            return m_side->vertices;
        }

        BrushFaceGeometry* BrushFace::side() const {
            return m_side;
        }

        void BrushFace::setSide(BrushFaceGeometry* side) {
            if (m_side == side)
                return;
            m_side = side;
            invalidateVertexCache();
        }
        
        void BrushFace::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
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
            
            mesh.beginTriangleSet(m_attribs.texture());
            mesh.addTrianglesToSet(m_cachedVertices);
            mesh.endTriangleSet();
        }
        
        Vec2f BrushFace::textureCoords(const Vec3& point) const {
            return m_texCoordSystem->getTexCoords(point, m_attribs);
        }
        
        void BrushFace::invalidate() {
            invalidateVertexCache();
        }

        FloatType BrushFace::intersectWithRay(const Ray3& ray) const {
            assert(m_side != NULL);
            
            const FloatType dot = m_boundary.normal.dot(ray.direction);
            if (!Math::neg(dot))
                return Math::nan<FloatType>();
            
            const FloatType dist = m_boundary.intersectWithRay(ray);
            if (Math::isnan(dist))
                return Math::nan<FloatType>();
            
            const size_t axis = m_boundary.normal.firstComponent();
            const Vec3 hit = ray.pointAtDistance(dist);
            const Vec3 projectedHit = swizzle(hit, axis);
            
            const BrushVertex* vertex = m_side->vertices.back();
            Vec3 v0 = swizzle(vertex->position, axis) - projectedHit;
            
            int c = 0;
            for (size_t i = 0; i < m_side->vertices.size(); i++) {
                vertex = m_side->vertices[i];
                const Vec3 v1 = swizzle(vertex->position, axis) - projectedHit;
                
                if ((Math::zero(v0.x()) && Math::zero(v0.y())) ||
                    (Math::zero(v1.x()) && Math::zero(v1.y()))) {
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
                return Math::nan<FloatType>();
            return dist;
        }
        
        void BrushFace::setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            m_points[0] = point0;
            m_points[1] = point1;
            m_points[2] = point2;
            correctPoints();
            
            if (!setPlanePoints(m_boundary, m_points)) {
                GeometryException e;
                e << "Colinear face points: (" <<
                m_points[0].asString() << ") (" <<
                m_points[1].asString() << ") (" <<
                m_points[2].asString() << ")";
                throw e;
            }
        }
        
        void BrushFace::correctPoints() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i].correct();
        }

        void BrushFace::validateVertexCache() const {
            m_cachedVertices.clear();
            
            const BrushVertexList& vertices = m_side->vertices;
            m_cachedVertices.reserve(3 * (vertices.size() - 2));
            
            for (size_t i = 1; i < vertices.size() - 1; i++) {
                m_cachedVertices.push_back(Vertex(vertices[0]->position,
                                                  m_boundary.normal,
                                                  textureCoords(vertices[0]->position)));
                m_cachedVertices.push_back(Vertex(vertices[i]->position,
                                                  m_boundary.normal,
                                                  textureCoords(vertices[i]->position)));
                m_cachedVertices.push_back(Vertex(vertices[i+1]->position,
                                                  m_boundary.normal,
                                                  textureCoords(vertices[i+1]->position)));
            }
            m_vertexCacheValid = true;
        }

        void BrushFace::invalidateVertexCache() {
            m_vertexCacheValid = false;
        }
    }
}
