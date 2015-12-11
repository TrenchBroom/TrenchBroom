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
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushFaceSnapshot.h"
#include "Model/PlanePointFinder.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/TexturedIndexArrayBuilder.h"

namespace TrenchBroom {
    namespace Model {
        const String BrushFace::NoTextureName = "__TB_empty";

        BrushFace::ProjectToVertex::Type BrushFace::ProjectToVertex::project(BrushHalfEdge* halfEdge) {
            return halfEdge->origin();
        }

        BrushFace::ProjectToEdge::Type BrushFace::ProjectToEdge::project(BrushHalfEdge* halfEdge) {
            return halfEdge->edge();
        }
        
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs, TexCoordSystem* texCoordSystem) :
        m_brush(NULL),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_texCoordSystem(texCoordSystem),
        m_geometry(NULL),
        m_vertexIndex(0),
        m_cachedVertices(0),
        m_verticesValid(false),
        m_attribs(attribs) {
            assert(m_texCoordSystem != NULL);
            setPoints(point0, point1, point2);
        }

        class PlaneWeightOrder {
        private:
            bool m_deterministic;
        public:
            PlaneWeightOrder(const bool deterministic) :
            m_deterministic(deterministic) {}

            template <typename T, size_t S>
            bool operator()(const Plane<T,S>& lhs, const Plane<T,S>& rhs) const {
                int result = lhs.normal.weight() - rhs.normal.weight();
                if (m_deterministic)
                    result += static_cast<int>(1000.0 * (lhs.distance - lhs.distance));
                return result < 0;
            }
        };

        class FaceWeightOrder {
        private:
            const PlaneWeightOrder& m_planeOrder;
        public:
            FaceWeightOrder(const PlaneWeightOrder& planeOrder) :
            m_planeOrder(planeOrder) {}

            bool operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const {
                return m_planeOrder(lhs->boundary(), rhs->boundary());
            }
        };

        BrushFace* BrushFace::createParaxial(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            const BrushFaceAttributes attribs(textureName);
            return new BrushFace(point0, point1, point2, attribs, new ParaxialTexCoordSystem(point0, point1, point2, attribs));
        }
        
        BrushFace* BrushFace::createParallel(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            const BrushFaceAttributes attribs(textureName);
            return new BrushFace(point0, point1, point2, attribs, new ParallelTexCoordSystem(point0, point1, point2, attribs));
        }
        
        void BrushFace::sortFaces(BrushFaceList& faces) {
            std::sort(faces.begin(), faces.end(), FaceWeightOrder(PlaneWeightOrder(true)));
            std::sort(faces.begin(), faces.end(), FaceWeightOrder(PlaneWeightOrder(false)));
        }

        BrushFace::~BrushFace() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i] = Vec3::Null;
            m_brush = NULL;
            m_lineNumber = 0;
            m_lineCount = 0;
            m_selected = false;
            delete m_texCoordSystem;
            m_texCoordSystem = NULL;
            m_geometry = NULL;
        }

        BrushFace* BrushFace::clone() const {
            BrushFace* result = new BrushFace(points()[0], points()[1], points()[2], textureName(), m_texCoordSystem->clone());
            result->m_attribs = m_attribs;
            result->setFilePosition(m_lineNumber, m_lineCount);
            if (m_selected)
                result->select();
            return result;
        }

        BrushFaceSnapshot* BrushFace::takeSnapshot() {
            return new BrushFaceSnapshot(this, m_texCoordSystem);
        }

        Brush* BrushFace::brush() const {
            return m_brush;
        }

        void BrushFace::setBrush(Brush* brush) {
            assert((m_brush == NULL) ^ (brush == NULL));
            m_brush = brush;
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
            assert(m_geometry != NULL);
            const BrushHalfEdgeList& boundary = m_geometry->boundary();
            return Vec3::center(boundary.begin(), boundary.end(), BrushGeometry::GetVertexPosition());
        }

        Vec3 BrushFace::boundsCenter() const {
            assert(m_geometry != NULL);

            const Mat4x4 toPlane = planeProjectionMatrix(m_boundary.distance, m_boundary.normal);
            const Mat4x4 fromPlane = invertedMatrix(toPlane);

            const BrushHalfEdge* first = m_geometry->boundary().front();
            const BrushHalfEdge* current = first;
            
            BBox3 bounds;
            bounds.min = bounds.max = toPlane * current->origin()->position();

            current = current->next();
            while (current != first) {
                bounds.mergeWith(toPlane * current->origin()->position());
                current = current->next();
            }
            return fromPlane * bounds.center();
        }

        FloatType BrushFace::area(const Math::Axis::Type axis) const {
            const BrushHalfEdge* first = m_geometry->boundary().front();
            const BrushHalfEdge* current = first;

            FloatType c1 = 0.0;
            FloatType c2 = 0.0;
            switch (axis) {
                case Math::Axis::AX:
                    do {
                        c1 += current->origin()->position().y() * current->next()->origin()->position().z();
                        c2 += current->origin()->position().z() * current->next()->origin()->position().y();
                        current = current->next();
                    } while (current != first);
                    break;
                case Math::Axis::AY:
                    do {
                        c1 += current->origin()->position().z() * current->next()->origin()->position().x();
                        c2 += current->origin()->position().x() * current->next()->origin()->position().z();
                        current = current->next();
                    } while (current != first);
                    break;
                case Math::Axis::AZ:
                    do {
                        c1 += current->origin()->position().x() * current->next()->origin()->position().y();
                        c2 += current->origin()->position().y() * current->next()->origin()->position().x();
                        current = current->next();
                    } while (current != first);
                    break;
            };
            return Math::abs((c1 - c2) / 2.0);
        }

        const BrushFaceAttributes& BrushFace::attribs() const {
            return m_attribs;
        }

        void BrushFace::setAttribs(const BrushFaceAttributes& attribs) {
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->decUsageCount();

            const float oldRotation = m_attribs.rotation();
            m_attribs = attribs;

            if (m_attribs.texture() != NULL)
                m_attribs.texture()->incUsageCount();

            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, m_attribs.rotation());

            if (m_brush != NULL)
                m_brush->faceDidChange();
            
            invalidateVertexCache();
        }

        const String& BrushFace::textureName() const {
            return m_attribs.textureName();
        }

        Assets::Texture* BrushFace::texture() const {
            return m_attribs.texture();
        }

        Vec2f BrushFace::textureSize() const {
            return m_attribs.textureSize();
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

        Vec2f BrushFace::modOffset(const Vec2f& offset) const {
            return m_attribs.modOffset(offset);
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

        bool BrushFace::hasSurfaceAttributes() const {
            return surfaceContents() != 0 || surfaceFlags() != 0 || surfaceValue() != 0.0f;
        }

        void BrushFace::updateTexture(Assets::TextureManager* textureManager) {
            assert(textureManager != NULL);
            Assets::Texture* texture = textureManager->texture(textureName());
            setTexture(texture);
            invalidateVertexCache();
        }

        void BrushFace::setTexture(Assets::Texture* texture) {
            if (texture == m_attribs.texture())
                return;
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->decUsageCount();
            m_attribs.setTexture(texture);
            if (m_attribs.texture() != NULL)
                m_attribs.texture()->incUsageCount();
            if (m_brush != NULL)
                m_brush->faceDidChange();
            invalidateVertexCache();
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
            if (m_brush != NULL)
                m_brush->faceDidChange();
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

        void BrushFace::setAttributes(const BrushFace* other) {
            setTexture(other->texture());
            setXOffset(other->xOffset());
            setYOffset(other->yOffset());
            setRotation(other->rotation());
            setXScale(other->xScale());
            setYScale(other->yScale());
            setSurfaceContents(other->surfaceContents());
            setSurfaceFlags(other->surfaceFlags());
            setSurfaceValue(other->surfaceValue());
        }

        Vec3 BrushFace::textureXAxis() const {
            return m_texCoordSystem->xAxis();
        }

        Vec3 BrushFace::textureYAxis() const {
            return m_texCoordSystem->yAxis();
        }

        void BrushFace::resetTextureAxes() {
            m_texCoordSystem->resetTextureAxes(m_boundary.normal);
            invalidateVertexCache();
        }

        void BrushFace::moveTexture(const Vec3& up, const Vec3& right, const Vec2f& offset) {
            m_texCoordSystem->moveTexture(m_boundary.normal, up, right, offset, m_attribs);
            invalidateVertexCache();
        }

        void BrushFace::rotateTexture(const float angle) {
            const float oldRotation = m_attribs.rotation();
            m_texCoordSystem->rotateTexture(m_boundary.normal, angle, m_attribs);
            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, m_attribs.rotation());
            invalidateVertexCache();
        }

        void BrushFace::shearTexture(const Vec2f& factors) {
            m_texCoordSystem->shearTexture(m_boundary.normal, factors);
            invalidateVertexCache();
        }

        void BrushFace::transform(const Mat4x4& transform, const bool lockTexture) {
            using std::swap;

            const Vec3 invariant = m_geometry != NULL ? center() : m_boundary.anchor();
            m_texCoordSystem->transform(m_boundary, transform, m_attribs, lockTexture, invariant);

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
            assert(m_geometry != NULL);

            // Find a triple of consecutive vertices s.t. the (normalized) vectors from the mid vertex to the other two
            // have the smallest dot value of all such triples. This is to have better precision when computing the
            // boundary plane normal from these vectors.
            FloatType bestDot = 1.0;
            const BrushHalfEdge* best = NULL;
            
            const BrushHalfEdge* first = m_geometry->boundary().front();
            const BrushHalfEdge* current = first;
            
            do {
                m_points[2] = current->next()->origin()->position();
                m_points[0] = current->origin()->position();
                m_points[1] = current->previous()->origin()->position();

                const Vec3 v1 = (m_points[2] - m_points[0]).normalized();
                const Vec3 v2 = (m_points[1] - m_points[0]).normalized();
                const FloatType dot = std::abs(v1.dot(v2));
                if (dot < bestDot) {
                    bestDot = dot;
                    best = current;
                }

                current = current->next();
            } while (current != first && bestDot > 0.0);

            assert(best != NULL);
            const Vec3 oldNormal = m_boundary.normal;
            setPoints(best->origin()->position(),
                      best->previous()->origin()->position(),
                      best->next()->origin()->position());

            m_texCoordSystem->updateNormal(oldNormal, m_boundary.normal, m_attribs);
        }

        void BrushFace::snapPlanePointsToInteger() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i].round();
            setPoints(m_points[0], m_points[1], m_points[2]);
        }

        void BrushFace::findIntegerPlanePoints() {
            PlanePointFinder::findPoints(m_boundary, m_points, 3);
            setPoints(m_points[0], m_points[1], m_points[2]);
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

        size_t BrushFace::vertexCount() const {
            assert(m_geometry != NULL);
            return m_geometry->boundary().size();
        }

        BrushFace::EdgeList BrushFace::edges() const {
            assert(m_geometry != NULL);
            return EdgeList(m_geometry->boundary());
        }

        BrushFace::VertexList BrushFace::vertices() const {
            assert(m_geometry != NULL);
            return VertexList(m_geometry->boundary());
        }

        BrushFaceGeometry* BrushFace::geometry() const {
            return m_geometry;
        }

        void BrushFace::setGeometry(BrushFaceGeometry* geometry) {
            if (m_geometry == geometry)
                return;
            m_geometry = geometry;
            invalidateVertexCache();
        }

        void BrushFace::invalidate() {
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
            assert(!m_selected);
            m_selected = true;
            if (m_brush != NULL)
                m_brush->childWasSelected();
        }

        void BrushFace::deselect() {
            assert(m_selected);
            m_selected = false;
            if (m_brush != NULL)
                m_brush->childWasDeselected();
        }

        void BrushFace::getVertices(Renderer::VertexListBuilder<VertexSpec>& builder) const {
            validateVertexCache();
            m_vertexIndex = builder.addPolygon(m_cachedVertices).index;

            GLuint index = static_cast<GLuint>(m_vertexIndex);
            // set the vertex indices
            const BrushHalfEdge* first = m_geometry->boundary().front();
            const BrushHalfEdge* current = first;
            do {
                BrushVertex* vertex = current->origin();
                vertex->setPayload(index++);
                
                // The boundary is in CCW order, but the renderer expects CW order:
                current = current->previous();
            } while (current != first);
        }
        
        void BrushFace::countIndices(Renderer::TexturedIndexArrayMap::Size& size) const {
            if (vertexCount() == 4)
                size.inc(texture(), GL_QUADS, 4);
            else
                size.inc(texture(), GL_TRIANGLES, 3 * (vertexCount() - 2));
        }

        void BrushFace::getFaceIndices(Renderer::TexturedIndexArrayBuilder& builder) const {
            if (vertexCount() == 4)
                builder.addQuads(texture(), static_cast<GLuint>(m_vertexIndex), vertexCount());
            else
                builder.addPolygon(texture(), static_cast<GLuint>(m_vertexIndex), vertexCount());
        }

        Vec2f BrushFace::textureCoords(const Vec3& point) const {
            return m_texCoordSystem->getTexCoords(point, m_attribs);
        }

        bool BrushFace::containsPoint(const Vec3& point) const {
            const Vec3 toPoint = point - m_boundary.anchor();
            if (!Math::zero(toPoint.dot(m_boundary.normal)))
                return false;

            const Ray3 ray(point + m_boundary.normal, -m_boundary.normal);
            return !Math::isnan(intersectWithRay(ray));
        }

        FloatType BrushFace::intersectWithRay(const Ray3& ray) const {
            assert(m_geometry != NULL);

            const FloatType dot = m_boundary.normal.dot(ray.direction);
            if (!Math::neg(dot))
                return Math::nan<FloatType>();
            
            return intersectPolygonWithRay(ray, m_boundary, m_geometry->boundary().begin(), m_geometry->boundary().end(), BrushGeometry::GetVertexPosition());
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

            invalidateVertexCache();
        }

        void BrushFace::correctPoints() {
            for (size_t i = 0; i < 3; ++i)
                m_points[i].correct();
        }

        bool BrushFace::vertexCacheValid() const {
            return m_verticesValid;
        }
        
        void BrushFace::invalidateVertexCache() {
            m_verticesValid = false;
        }
        
        void BrushFace::validateVertexCache() const {
            if (!m_verticesValid) {
                m_cachedVertices.clear();
                m_cachedVertices.reserve(vertexCount());
                
                const BrushHalfEdge* first = m_geometry->boundary().front();
                const BrushHalfEdge* current = first;
                do {
                    const Vec3& position = current->origin()->position();
                    m_cachedVertices.push_back(Vertex(position, m_boundary.normal, textureCoords(position)));
                    
                    // The boundary is in CCW order, but the renderer expects CW order:
                    current = current->previous();
                } while (current != first);
                
                m_verticesValid = true;
            }
        }
    }
}
