/*
 Copyright (C) 2010-2017 Kristian Duske

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
        
        BrushFace::BrushFace(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs, TexCoordSystem* texCoordSystem) :
        m_brush(nullptr),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_texCoordSystem(texCoordSystem),
        m_geometry(nullptr),
        m_attribs(attribs) {
            ensure(m_texCoordSystem != nullptr, "texCoordSystem is null");
            setPoints(point0, point1, point2);
        }

        class FaceWeightOrder {
        private:
            bool m_deterministic;
        public:
            FaceWeightOrder(const bool deterministic) :
            m_deterministic(deterministic) {}

            bool operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const {
                const auto& lhsBoundary = lhs->boundary();
                const auto& rhsBoundary = rhs->boundary();
                auto result = weight(lhsBoundary.normal) - weight(rhsBoundary.normal);
                if (m_deterministic) {
                    result += static_cast<int>(1000.0 * (lhsBoundary.distance - lhsBoundary.distance));
                }

                return result < 0;
            }
        private:
            template <typename T>
            int weight(const vec<T,3>& vec) const {
                return weight(vec[0]) * 100 + weight(vec[1]) * 10 + weight(vec[2]);
            }

            template <typename T>
            int weight(T c) const {
                if (std::abs(c - static_cast<T>(1.0)) < static_cast<T>(0.9))
                    return 0;
                if (std::abs(c + static_cast<T>(1.0)) < static_cast<T>(0.9))
                    return 1;
                return 2;
            }
        };

        BrushFace* BrushFace::createParaxial(const vec3& point0, const vec3& point1, const vec3& point2, const String& textureName) {
            const BrushFaceAttributes attribs(textureName);
            return new BrushFace(point0, point1, point2, attribs, new ParaxialTexCoordSystem(point0, point1, point2, attribs));
        }
        
        BrushFace* BrushFace::createParallel(const vec3& point0, const vec3& point1, const vec3& point2, const String& textureName) {
            const BrushFaceAttributes attribs(textureName);
            return new BrushFace(point0, point1, point2, attribs, new ParallelTexCoordSystem(point0, point1, point2, attribs));
        }
        
        void BrushFace::sortFaces(BrushFaceList& faces) {
            std::sort(std::begin(faces), std::end(faces), FaceWeightOrder(true));
            std::sort(std::begin(faces), std::end(faces), FaceWeightOrder(false));
        }

        BrushFace::~BrushFace() {
            for (size_t i = 0; i < 3; ++i) {
                m_points[i] = vec3::zero;
            }
            m_brush = nullptr;
            m_lineNumber = 0;
            m_lineCount = 0;
            m_selected = false;
            delete m_texCoordSystem;
            m_texCoordSystem = nullptr;
            m_geometry = nullptr;
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
        
        TexCoordSystemSnapshot* BrushFace::takeTexCoordSystemSnapshot() const {
            return m_texCoordSystem->takeSnapshot();
        }
        
        void BrushFace::restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot* coordSystemSnapshot) {
            coordSystemSnapshot->restore(m_texCoordSystem);
        }

        void BrushFace::copyTexCoordSystemFromFace(const TexCoordSystemSnapshot* coordSystemSnapshot, const BrushFaceAttributes& attribs, const plane3& sourceFacePlane, const WrapStyle wrapStyle) {
            // Get a line, and a reference point, that are on both the source face's plane and our plane
            const auto seam = intersect(sourceFacePlane, m_boundary);
            const auto refPoint = seam.projectPoint(center());
            
            coordSystemSnapshot->restore(m_texCoordSystem);
            
            // Get the texcoords at the refPoint using the source face's attribs and tex coord system
            const auto desriedCoords = m_texCoordSystem->getTexCoords(refPoint, attribs) * attribs.textureSize();
            
            m_texCoordSystem->updateNormal(sourceFacePlane.normal, m_boundary.normal, m_attribs, wrapStyle);
            
            // Adjust the offset on this face so that the texture coordinates at the refPoint stay the same
            if (!isZero(seam.direction)) {
                const auto currentCoords = m_texCoordSystem->getTexCoords(refPoint, m_attribs) * m_attribs.textureSize();
                const auto offsetChange = desriedCoords - currentCoords;
                m_attribs.setOffset(correct(m_attribs.modOffset(m_attribs.offset() + offsetChange), 4));
            }
        }
        
        Brush* BrushFace::brush() const {
            return m_brush;
        }

        void BrushFace::setBrush(Brush* brush) {
            assert((m_brush == nullptr) ^ (brush == nullptr));
            m_brush = brush;
        }

        const BrushFace::Points& BrushFace::points() const {
            return m_points;
        }

        bool BrushFace::arePointsOnPlane(const plane3& plane) const {
            for (size_t i = 0; i < 3; i++)
                if (plane.pointStatus(m_points[i]) != Math::PointStatus::PSInside)
                    return false;
            return true;
        }

        const plane3& BrushFace::boundary() const {
            return m_boundary;
        }

        const vec3& BrushFace::normal() const {
            return boundary().normal;
        }

        vec3 BrushFace::center() const {
            ensure(m_geometry != nullptr, "geometry is null");
            const BrushHalfEdgeList& boundary = m_geometry->boundary();
            return average(std::begin(boundary), std::end(boundary), BrushGeometry::GetVertexPosition());
        }

        vec3 BrushFace::boundsCenter() const {
            ensure(m_geometry != nullptr, "geometry is null");

            const auto toPlane = planeProjectionMatrix(m_boundary.distance, m_boundary.normal);
            const auto [invertible, fromPlane] = ::invert(toPlane);
            assert(invertible); unused(invertible);

            const auto* first = m_geometry->boundary().front();
            const auto* current = first;
            
            bbox3 bounds;
            bounds.min = bounds.max = toPlane * current->origin()->position();

            current = current->next();
            while (current != first) {
                bounds = merge(bounds, toPlane * current->origin()->position());
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
            const float oldRotation = m_attribs.rotation();
            m_attribs = attribs;
            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, m_attribs.rotation());

            if (m_brush != nullptr)
                m_brush->faceDidChange();
            
            invalidateVertexCache();
        }

        void BrushFace::resetTexCoordSystemCache() {
            if (m_texCoordSystem != nullptr) {
                m_texCoordSystem->resetCache(m_points[0], m_points[1], m_points[2], m_attribs);
            }
        }

        const String& BrushFace::textureName() const {
            return m_attribs.textureName();
        }

        Assets::Texture* BrushFace::texture() const {
            return m_attribs.texture();
        }

        vec2f BrushFace::textureSize() const {
            return m_attribs.textureSize();
        }

        const vec2f& BrushFace::offset() const {
            return m_attribs.offset();
        }

        float BrushFace::xOffset() const {
            return m_attribs.xOffset();
        }

        float BrushFace::yOffset() const {
            return m_attribs.yOffset();
        }

        vec2f BrushFace::modOffset(const vec2f& offset) const {
            return m_attribs.modOffset(offset);
        }

        const vec2f& BrushFace::scale() const {
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
            ensure(textureManager != nullptr, "textureManager is null");
            Assets::Texture* texture = textureManager->texture(textureName());
            setTexture(texture);
            invalidateVertexCache();
        }

        void BrushFace::setTexture(Assets::Texture* texture) {
            if (texture == m_attribs.texture())
                return;
            m_attribs.setTexture(texture);
            if (m_brush != nullptr)
                m_brush->faceDidChange();
            invalidateVertexCache();
        }

        void BrushFace::unsetTexture() {
            if (m_attribs.texture() == nullptr)
                return;
            m_attribs.unsetTexture();
            if (m_brush != nullptr)
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
            if (m_brush != nullptr)
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

        vec3 BrushFace::textureXAxis() const {
            return m_texCoordSystem->xAxis();
        }

        vec3 BrushFace::textureYAxis() const {
            return m_texCoordSystem->yAxis();
        }

        void BrushFace::resetTextureAxes() {
            m_texCoordSystem->resetTextureAxes(m_boundary.normal);
            invalidateVertexCache();
        }

        void BrushFace::moveTexture(const vec3& up, const vec3& right, const vec2f& offset) {
            m_texCoordSystem->moveTexture(m_boundary.normal, up, right, offset, m_attribs);
            invalidateVertexCache();
        }

        void BrushFace::rotateTexture(const float angle) {
            const float oldRotation = m_attribs.rotation();
            m_texCoordSystem->rotateTexture(m_boundary.normal, angle, m_attribs);
            m_texCoordSystem->setRotation(m_boundary.normal, oldRotation, m_attribs.rotation());
            invalidateVertexCache();
        }

        void BrushFace::shearTexture(const vec2f& factors) {
            m_texCoordSystem->shearTexture(m_boundary.normal, factors);
            invalidateVertexCache();
        }

        void BrushFace::transform(const mat4x4& transform, const bool lockTexture) {
            using std::swap;

            const vec3 invariant = m_geometry != nullptr ? center() : m_boundary.anchor();
            const plane3 oldBoundary = m_boundary;

            m_boundary = m_boundary.transform(transform);
            for (size_t i = 0; i < 3; ++i) {
                m_points[i] = transform * m_points[i];
            }
            
            if (dot(cross(m_points[2] - m_points[0], m_points[1] - m_points[0]), m_boundary.normal) < 0.0) {
                swap(m_points[1], m_points[2]);
            }

            setPoints(m_points[0], m_points[1], m_points[2]);
            
            m_texCoordSystem->transform(oldBoundary, m_boundary, transform, m_attribs, lockTexture, invariant);
        }

        void BrushFace::invert() {
            using std::swap;

            m_boundary = m_boundary.flip();
            swap(m_points[1], m_points[2]);
            invalidateVertexCache();
        }

        void BrushFace::updatePointsFromVertices() {
            ensure(m_geometry != nullptr, "geometry is null");

            const auto* first = m_geometry->boundary().front();
            const auto oldPlane = m_boundary;
            setPoints(first->next()->origin()->position(),
                      first->origin()->position(),
                      first->previous()->origin()->position());

            // Get a line, and a reference point, that are on both the old plane
            // (before moving the face) and after moving the face.
            const auto seam = intersect(oldPlane, m_boundary);
            if (!isZero(seam.direction)) {
                const auto refPoint = seam.projectPoint(center());
                
                // Get the texcoords at the refPoint using the old face's attribs and tex coord system
                const auto desriedCoords = m_texCoordSystem->getTexCoords(refPoint, m_attribs) * m_attribs.textureSize();
                
                m_texCoordSystem->updateNormal(oldPlane.normal, m_boundary.normal, m_attribs, WrapStyle::Projection);
                
                // Adjust the offset on this face so that the texture coordinates at the refPoint stay the same
                const auto currentCoords = m_texCoordSystem->getTexCoords(refPoint, m_attribs) * m_attribs.textureSize();
                const auto offsetChange = desriedCoords - currentCoords;
                m_attribs.setOffset(correct(m_attribs.modOffset(m_attribs.offset() + offsetChange), 4));
            }
        }

        void BrushFace::snapPlanePointsToInteger() {
            for (size_t i = 0; i < 3; ++i) {
                m_points[i] = round(m_points[i]);
            }
            setPoints(m_points[0], m_points[1], m_points[2]);
        }

        void BrushFace::findIntegerPlanePoints() {
            PlanePointFinder::findPoints(m_boundary, m_points, 3);
            setPoints(m_points[0], m_points[1], m_points[2]);
        }

        mat4x4 BrushFace::projectToBoundaryMatrix() const {
            const auto texZAxis = m_texCoordSystem->fromMatrix(vec2f::zero, vec2f::one) * vec3::pos_z;
            const auto worldToPlaneMatrix = planeProjectionMatrix(m_boundary.distance, m_boundary.normal, texZAxis);
            const auto [invertible, planeToWorldMatrix] = ::invert(worldToPlaneMatrix); assert(invertible); unused(invertible);
            return planeToWorldMatrix * mat4x4::zero_z * worldToPlaneMatrix;
        }

        mat4x4 BrushFace::toTexCoordSystemMatrix(const vec2f& offset, const vec2f& scale, const bool project) const {
            if (project) {
                return mat4x4::zero_z * m_texCoordSystem->toMatrix(offset, scale);
            } else {
                return m_texCoordSystem->toMatrix(offset, scale);
            }
        }

        mat4x4 BrushFace::fromTexCoordSystemMatrix(const vec2f& offset, const vec2f& scale, const bool project) const {
            if (project) {
                return projectToBoundaryMatrix() * m_texCoordSystem->fromMatrix(offset, scale);
            } else {
                return m_texCoordSystem->fromMatrix(offset, scale);
            }
        }

        float BrushFace::measureTextureAngle(const vec2f& center, const vec2f& point) const {
            return m_texCoordSystem->measureAngle(m_attribs.rotation(), center, point);
        }

        size_t BrushFace::vertexCount() const {
            assert(m_geometry != nullptr);
            return m_geometry->boundary().size();
        }

        BrushFace::EdgeList BrushFace::edges() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return EdgeList(m_geometry->boundary());
        }

        BrushFace::VertexList BrushFace::vertices() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return VertexList(m_geometry->boundary());
        }

        vec3::List BrushFace::vertexPositions() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->vertexPositions();
        }

        bool BrushFace::hasVertices(const Polygon3& vertices, const FloatType epsilon) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->hasVertexPositions(vertices.vertices(), epsilon);
        }

        Polygon3 BrushFace::polygon() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return Polygon3(vertexPositions());
        }

        BrushFaceGeometry* BrushFace::geometry() const {
            return m_geometry;
        }

        void BrushFace::setGeometry(BrushFaceGeometry* geometry) {
            if (m_geometry != nullptr) {
                m_geometry->setPayload(nullptr);
            }
            m_geometry = geometry;
            if (m_geometry != nullptr) {
                m_geometry->setPayload(this);
            }
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
            if (m_brush != nullptr)
                m_brush->childWasSelected();
        }

        void BrushFace::deselect() {
            assert(m_selected);
            m_selected = false;
            if (m_brush != nullptr)
                m_brush->childWasDeselected();
        }

        vec2f BrushFace::textureCoords(const vec3& point) const {
            return m_texCoordSystem->getTexCoords(point, m_attribs);
        }

        bool BrushFace::containsPoint(const vec3& point) const {
            const vec3 toPoint = point - m_boundary.anchor();
            if (!Math::zero(dot(toPoint, m_boundary.normal)))
                return false;

            const ray3 ray(point + m_boundary.normal, -m_boundary.normal);
            return !Math::isnan(intersectWithRay(ray));
        }

        FloatType BrushFace::intersectWithRay(const ray3& ray) const {
            ensure(m_geometry != nullptr, "geometry is null");

            const FloatType cos = dot(m_boundary.normal, ray.direction);
            if (!Math::neg(cos))
                return Math::nan<FloatType>();
            
            return intersectPolygonWithRay(ray, m_boundary, m_geometry->boundary().begin(), m_geometry->boundary().end(), BrushGeometry::GetVertexPosition());
        }

        void BrushFace::printPoints() const {
            std::for_each(std::begin(m_points), std::end(m_points), [](const vec3& p) { std::cout << "( " << p << " ) "; });
            std::cout << std::endl;
        }

        void BrushFace::setPoints(const vec3& point0, const vec3& point1, const vec3& point2) {
            m_points[0] = point0;
            m_points[1] = point1;
            m_points[2] = point2;
            correctPoints();

            const auto [result, plane] = fromPoints(m_points[0], m_points[1], m_points[2]);
            if (!result) {
                GeometryException e;
                e << "Colinear face points: (" <<
                m_points[0] << ") (" <<
                m_points[1] << ") (" <<
                m_points[2] << ")";
                throw e;
            } else {
                m_boundary = plane;
            }

            invalidateVertexCache();
        }

        void BrushFace::correctPoints() {
            for (size_t i = 0; i < 3; ++i) {
                m_points[i] = correct(m_points[i]);
            }
        }

        void BrushFace::invalidateVertexCache() {
            if (m_brush != nullptr) {
                m_brush->invalidateVertexCache();
            }
        }

        void BrushFace::setMarked(const bool marked) const {
            m_markedToRenderFace = marked;
        }

        bool BrushFace::isMarked() const {
            return m_markedToRenderFace;
        }
    }
}
