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

#include "Brush.h"

#include "Exceptions.h"
#include "FloatType.h"
#include "Polyhedron.h"
#include "Polyhedron_Matcher.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/ModelFactory.h"
#include "Model/TexCoordSystem.h"

#include <kdl/vector_utils.h>

#include <vecmath/intersection.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/segment.h>
#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <algorithm> // for std::remove
#include <iterator>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

namespace TrenchBroom {
    namespace Model {
        Brush::Brush() :
        m_transparent(false) {}

        Brush::Brush(const vm::bbox3& worldBounds, std::vector<BrushFace> faces) :
        m_faces(std::move(faces)),
        m_transparent(false) {
            updateGeometryFromFaces(worldBounds);
        }

        class Brush::CopyCallback : public BrushGeometry::CopyCallback {
        public:
            void faceWasCopied(const BrushFaceGeometry* original, BrushFaceGeometry* copy) const override {
                copy->setPayload(original->payload());
            }
        };

        Brush::Brush(const Brush& other) :
        m_faces(other.m_faces),
        m_geometry(other.m_geometry ? std::make_unique<BrushGeometry>(*other.m_geometry, CopyCallback()) : nullptr),
        m_transparent(other.m_transparent) {
            if (m_geometry) {
                for (BrushFaceGeometry* faceGeometry : m_geometry->faces()) {
                    if (const auto faceIndex = faceGeometry->payload()) {
                        BrushFace& face = m_faces[*faceIndex];
                        face.setGeometry(faceGeometry);
                    }
                }
            }
        }

        Brush::Brush(Brush&& other) noexcept :
        m_faces(std::move(other.m_faces)),
        m_geometry(std::move(other.m_geometry)),
        m_transparent(other.m_transparent) {}

        Brush& Brush::operator=(Brush other) noexcept {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(Brush& lhs, Brush& rhs) noexcept {
            using std::swap;
            swap(lhs.m_faces, rhs.m_faces);
            swap(lhs.m_geometry, rhs.m_geometry);
            swap(lhs.m_transparent, rhs.m_transparent);
        }
        
        Brush::~Brush() = default;

        void Brush::updateGeometryFromFaces(const vm::bbox3& worldBounds) {
            // First, add all faces to the brush geometry
            BrushFace::sortFaces(m_faces);
            
            auto geometry = std::make_unique<BrushGeometry>(worldBounds);
            
            for (size_t i = 0u; i < m_faces.size(); ++i) {
                BrushFace& face = m_faces[i];
                const auto result = geometry->clip(face.boundary());
                if (result.success()) {
                    BrushFaceGeometry* faceGeometry = result.face();
                    face.setGeometry(faceGeometry);
                    faceGeometry->setPayload(i);
                } else  if (result.empty()) {
                    throw GeometryException("Brush is empty");
                }
            }

            // Correct vertex positions and heal short edges
            geometry->correctVertexPositions();
            if (!geometry->healEdges()) {
                throw GeometryException("Brush is invalid");
            }
            
            // Now collect all faces which still remain
            std::vector<BrushFace> remainingFaces;
            remainingFaces.reserve(m_faces.size());
            
            for (BrushFaceGeometry* faceGeometry : geometry->faces()) {
                if (const auto faceIndex = faceGeometry->payload()) {
                    remainingFaces.push_back(std::move(m_faces[*faceIndex]));
                    faceGeometry->setPayload(remainingFaces.size() - 1u);
                } else {
                    throw GeometryException("Brush is not fully specified");
                }
            }

            m_faces = std::move(remainingFaces);
            m_geometry = std::move(geometry);
            
            assert(checkFaceLinks());
        }
        
        const vm::bbox3& Brush::bounds() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->bounds();
        }

        std::optional<size_t> Brush::findFace(const std::string& textureName) const {
            return kdl::vec_index_of(m_faces, [&](const BrushFace& face) { return face.attributes().textureName() == textureName; });
        }

        std::optional<size_t> Brush::findFace(const vm::vec3& normal) const {
            return kdl::vec_index_of(m_faces, [&](const BrushFace& face) { return vm::is_equal(face.boundary().normal, normal, vm::C::almost_zero()); });
        }

        std::optional<size_t> Brush::findFace(const vm::plane3& boundary) const {
            return kdl::vec_index_of(m_faces, [&](const BrushFace& face) { return vm::is_equal(face.boundary(), boundary, vm::C::almost_zero()); });
        }

        std::optional<size_t> Brush::findFace(const vm::polygon3& vertices, const FloatType epsilon) const {
            return kdl::vec_index_of(m_faces, [&](const BrushFace& face) { return face.hasVertices(vertices, epsilon); });
        }

        std::optional<size_t> Brush::findFace(const std::vector<vm::polygon3>& candidates, const FloatType epsilon) const {
            for (const auto& candidate : candidates) {
                if (const auto faceIndex = findFace(candidate, epsilon)) {
                    return faceIndex;
                }
            }
            return std::nullopt;
        }

        const BrushFace& Brush::face(const size_t index) const {
            assert(index < faceCount());
            return m_faces[index];
        }

        BrushFace& Brush::face(const size_t index) {
            assert(index < faceCount());
            return m_faces[index];
        }

        size_t Brush::faceCount() const {
            return m_faces.size();
        }

        const std::vector<BrushFace>& Brush::faces() const {
            return m_faces;
        }

        std::vector<BrushFace>& Brush::faces() {
            return m_faces;
        }

        bool Brush::closed() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->closed();
        }

        bool Brush::fullySpecified() const {
            ensure(m_geometry != nullptr, "geometry is null");

            for (auto* current : m_geometry->faces()) {
                if (!current->payload().has_value()) {
                    return false;
                }
            }
            return true;
        }

        void Brush::cloneFaceAttributesFrom(const Brush& brush) {
            for (auto& destination : m_faces) {
                if (const auto sourceIndex = brush.findFace(destination.boundary())) {
                    const auto& source = brush.face(*sourceIndex);
                    destination.setAttributes(source.attributes());

                    auto snapshot = source.takeTexCoordSystemSnapshot();
                    if (snapshot != nullptr) {
                        destination.copyTexCoordSystemFromFace(*snapshot, source.attributes().takeSnapshot(), source.boundary(), WrapStyle::Projection);
                    }
                }
            }
        }

        void Brush::cloneInvertedFaceAttributesFrom(const Brush& brush) {
            for (auto& destination : m_faces) {
                if (const auto sourceIndex = brush.findFace(destination.boundary().flip())) {
                    const auto& source = brush.face(*sourceIndex);
                    // Todo: invert the face attributes?
                    destination.setAttributes(source.attributes());

                    auto snapshot = source.takeTexCoordSystemSnapshot();
                    if (snapshot != nullptr) {
                        destination.copyTexCoordSystemFromFace(*snapshot, source.attributes().takeSnapshot(), destination.boundary(), WrapStyle::Projection);
                    }
                }
            }
        }

        bool Brush::clip(const vm::bbox3& worldBounds, BrushFace face) {
            try {
                m_faces.push_back(std::move(face));
                updateGeometryFromFaces(worldBounds);
                return !m_faces.empty();
            } catch (GeometryException&) {
                return false;
            }
        }

        bool Brush::canMoveBoundary(const vm::bbox3& worldBounds, const size_t faceIndex, const vm::vec3& delta) const {
            const auto& face = this->face(faceIndex);
            auto testFace = BrushFace(face);
            testFace.transform(vm::translation_matrix(delta), false);

            std::vector<BrushFace> testFaces;
            testFaces.reserve(faceCount());
            testFaces.push_back(std::move(testFace));

            for (const auto& brushFace : m_faces) {
                if (&brushFace != &face) {
                    testFaces.emplace_back(brushFace);
                }
            }

            try {
                const auto testBrush = Brush(worldBounds, std::move(testFaces));
                const auto inWorldBounds = worldBounds.contains(testBrush.bounds());
                const auto closed = testBrush.closed();
                const auto allFaces = testBrush.faceCount() == faceCount();

                return inWorldBounds && closed && allFaces;
            } catch (const GeometryException&) {
                return false;
            }
        }

        void Brush::moveBoundary(const vm::bbox3& worldBounds, const size_t faceIndex, const vm::vec3& delta, const bool lockTexture) {
            assert(canMoveBoundary(worldBounds, faceIndex, delta));

            auto& face = this->face(faceIndex);
            face.transform(vm::translation_matrix(delta), lockTexture);
            updateGeometryFromFaces(worldBounds);
        }

        bool Brush::canExpand(const vm::bbox3& worldBounds, const FloatType delta, const bool lockTexture) const {
            auto testBrush = Brush(*this);
            return testBrush.expand(worldBounds, delta, lockTexture);
        }

        bool Brush::expand(const vm::bbox3& worldBounds, const FloatType delta, const bool lockTexture) {
            // move the faces
            for (BrushFace& face : m_faces) {
                const vm::vec3 moveAmount = face.boundary().normal * delta;
                face.transform(vm::translation_matrix(moveAmount), lockTexture);
            }

            // rebuild geometry
            try {
                updateGeometryFromFaces(worldBounds);
                return !m_faces.empty();
            } catch (GeometryException&) {
                return false;
            }
        }

        size_t Brush::vertexCount() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->vertexCount();
        }

        const Brush::VertexList& Brush::vertices() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->vertices();
        }

        const std::vector<vm::vec3> Brush::vertexPositions() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->vertexPositions();
        }

        bool Brush::hasVertex(const vm::vec3& position, const FloatType epsilon) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->findVertexByPosition(position, epsilon) != nullptr;
        }

        vm::vec3 Brush::findClosestVertexPosition(const vm::vec3& position) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->findClosestVertex(position)->position();
        }

        bool Brush::hasEdge(const vm::segment3& edge, const FloatType epsilon) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->findEdgeByPositions(edge.start(), edge.end(), epsilon) != nullptr;
        }

        bool Brush::hasFace(const vm::polygon3& face, const FloatType epsilon) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->hasFace(face.vertices(), epsilon);
        }

        size_t Brush::edgeCount() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->edgeCount();
        }

        const Brush::EdgeList& Brush::edges() const {
            ensure(m_geometry != nullptr, "geometry is null");
            return m_geometry->edges();
        }

        bool Brush::containsPoint(const vm::vec3& point) const {
            if (!bounds().contains(point)) {
                return false;
            } else {
                for (const auto& face : m_faces) {
                    if (face.boundary().point_status(point) == vm::plane_status::above) {
                        return false;
                    }
                }
                return true;
            }
        }

        std::vector<const BrushFace*> Brush::incidentFaces(const BrushVertex* vertex) const {
            std::vector<const BrushFace*> result;
            result.reserve(m_faces.size());

            auto* first = vertex->leaving();
            auto* current = first;
            do {
                if (const auto faceIndex = current->face()->payload()) {
                    result.push_back(&m_faces[*faceIndex]);
                }
                current = current->nextIncident();
            } while (current != first);

            return result;
        }

        bool Brush::canMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertices, const vm::vec3& delta) const {
            return doCanMoveVertices(worldBounds, vertices, delta, true).success;
        }

        std::vector<vm::vec3> Brush::moveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, const bool uvLock) {
            doMoveVertices(worldBounds, vertexPositions, delta, uvLock);

            // Collect the exact new positions of the moved vertices
            std::vector<vm::vec3> result;
            result.reserve(vertexPositions.size());

            for (const auto& position : vertexPositions) {
                const auto* newVertex = m_geometry->findClosestVertex(position + delta, vm::C::almost_zero());
                if (newVertex != nullptr) {
                    result.push_back(newVertex->position());
                }
            }

            return result;
        }

        bool Brush::canAddVertex(const vm::bbox3& worldBounds, const vm::vec3& position) const {
            ensure(m_geometry != nullptr, "geometry is null");
            if (!worldBounds.contains(position)) {
                return false;
            }
            
            BrushGeometry newGeometry(kdl::vec_concat(m_geometry->vertexPositions(), std::vector<vm::vec3>({position})));
            return newGeometry.hasVertex(position);
        }

        BrushVertex* Brush::addVertex(const vm::bbox3& worldBounds, const vm::vec3& position) {
            assert(canAddVertex(worldBounds, position));

            BrushGeometry newGeometry(kdl::vec_concat(m_geometry->vertexPositions(), std::vector<vm::vec3>({position})));

            const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry);
            doSetNewGeometry(worldBounds, matcher, newGeometry);

            auto* newVertex = m_geometry->findClosestVertex(position, vm::C::almost_zero());
            ensure(newVertex != nullptr, "vertex could not be added");
            return newVertex;
        }

        namespace {
            BrushGeometry removeVerticesFromGeometry(const BrushGeometry& geometry, const std::vector<vm::vec3>& vertexPositions) {
                std::vector<vm::vec3> points;
                points.reserve(geometry.vertexCount());
                
                for (const auto* vertex : geometry.vertices()) {
                    const auto& position = vertex->position();
                    if (!kdl::vec_contains(vertexPositions, position)) {
                        points.push_back(position);
                    }
                }
                
                return BrushGeometry(points);
            }
        }

        bool Brush::canRemoveVertices(const vm::bbox3& /* worldBounds */, const std::vector<vm::vec3>& vertexPositions) const {
            ensure(m_geometry != nullptr, "geometry is null");
            ensure(!vertexPositions.empty(), "no vertex positions");

            return removeVerticesFromGeometry(*m_geometry, vertexPositions).polyhedron();
        }

        void Brush::removeVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions) {
            ensure(m_geometry != nullptr, "geometry is null");
            ensure(!vertexPositions.empty(), "no vertex positions");
            assert(canRemoveVertices(worldBounds, vertexPositions));

            const BrushGeometry newGeometry = removeVerticesFromGeometry(*m_geometry, vertexPositions);
            const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry);
            doSetNewGeometry(worldBounds, matcher, newGeometry);
        }

        namespace {
            BrushGeometry snappedGeometry(const BrushGeometry& geometry, const FloatType snapToF) {
                std::vector<vm::vec3> points;
                points.reserve(geometry.vertexCount());
                
                for (const auto* vertex : geometry.vertices()) {
                    points.push_back(snapToF * vm::round(vertex->position() / snapToF));
                }

                return BrushGeometry(std::move(points));
            }
        }
        
        bool Brush::canSnapVertices(const vm::bbox3& /* worldBounds */, const FloatType snapToF) const {
            ensure(m_geometry != nullptr, "geometry is null");
            return snappedGeometry(*m_geometry, snapToF).polyhedron();
        }

        void Brush::snapVertices(const vm::bbox3& worldBounds, const FloatType snapToF, const bool uvLock) {
            ensure(m_geometry != nullptr, "geometry is null");
            
            const BrushGeometry newGeometry = snappedGeometry(*m_geometry, snapToF);

            std::map<vm::vec3,vm::vec3> vertexMapping;
            for (const auto* vertex : m_geometry->vertices()) {
                const auto& origin = vertex->position();
                const auto destination = snapToF * round(origin / snapToF);
                if (newGeometry.hasVertex(destination)) {
                    vertexMapping.insert(std::make_pair(origin, destination));
                }
            }

            const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry, vertexMapping);
            doSetNewGeometry(worldBounds, matcher, newGeometry, uvLock);
        }

        bool Brush::canMoveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta) const {
            ensure(m_geometry != nullptr, "geometry is null");
            ensure(!edgePositions.empty(), "no edge positions");

            std::vector<vm::vec3> vertexPositions;
            vm::segment3::get_vertices(
                std::begin(edgePositions), std::end(edgePositions),
                std::back_inserter(vertexPositions));
            const auto result = doCanMoveVertices(worldBounds, vertexPositions, delta, false);

            if (!result.success) {
                return false;
            }

            for (const auto& edge : edgePositions) {
                if (!result.geometry->hasEdge(edge.start() + delta, edge.end() + delta)) {
                    return false;
                }
            }

            return true;
        }

        std::vector<vm::segment3> Brush::moveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta, const bool uvLock) {
            assert(canMoveEdges(worldBounds, edgePositions, delta));

            std::vector<vm::vec3> vertexPositions;
            vm::segment3::get_vertices(std::begin(edgePositions), std::end(edgePositions),
                                  std::back_inserter(vertexPositions));
            doMoveVertices(worldBounds, vertexPositions, delta, uvLock);

            std::vector<vm::segment3> result;
            result.reserve(edgePositions.size());

            for (const auto& edgePosition : edgePositions) {
                const auto* newEdge = m_geometry->findClosestEdge(edgePosition.start() + delta, edgePosition.end() + delta,
                    vm::C::almost_zero());
                if (newEdge != nullptr) {
                    result.push_back(vm::segment3(newEdge->firstVertex()->position(), newEdge->secondVertex()->position()));
                }
            }

            return result;
        }

        bool Brush::canMoveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta) const {
            ensure(m_geometry != nullptr, "geometry is null");
            ensure(!facePositions.empty(), "no face positions");

            std::vector<vm::vec3> vertexPositions;
            vm::polygon3::get_vertices(std::begin(facePositions), std::end(facePositions), std::back_inserter(vertexPositions));
            const auto result = doCanMoveVertices(worldBounds, vertexPositions, delta, false);

            if (!result.success) {
                return false;
            }

            for (const auto& face : facePositions) {
                if (!result.geometry->hasFace(face.vertices() + delta)) {
                    return false;
                }
            }

            return true;
        }

        std::vector<vm::polygon3> Brush::moveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta, const bool uvLock) {
            assert(canMoveFaces(worldBounds, facePositions, delta));

            std::vector<vm::vec3> vertexPositions;
            vm::polygon3::get_vertices(std::begin(facePositions), std::end(facePositions), std::back_inserter(vertexPositions));
            doMoveVertices(worldBounds, vertexPositions, delta, uvLock);

            std::vector<vm::polygon3> result;
            result.reserve(facePositions.size());

            for (const auto& facePosition : facePositions) {
                const auto* newFace = m_geometry->findClosestFace(facePosition.vertices() + delta, vm::C::almost_zero());
                if (newFace != nullptr) {
                    result.push_back(vm::polygon3(newFace->vertexPositions()));
                }
            }

            return result;
        }

        Brush::CanMoveVerticesResult::CanMoveVerticesResult(const bool s, BrushGeometry&& g) :
        success(s),
        geometry(std::make_unique<BrushGeometry>(std::move(g))) {}

        Brush::CanMoveVerticesResult Brush::CanMoveVerticesResult::rejectVertexMove() {
            return CanMoveVerticesResult(false, BrushGeometry());
        }

        Brush::CanMoveVerticesResult Brush::CanMoveVerticesResult::acceptVertexMove(BrushGeometry&& result) {
            return CanMoveVerticesResult(true, std::move(result));
        }

        /*
         We determine whether a move is valid by considering the vertices being moved and the vertices
         remaining at their positions as polyhedra. Depending on whether or not they really are polyhedra,
         polygons, edges, points, or empty, we have to consider the following cases.

         REMAINING  || Empty   | Point  | Edge   | Polygon | Polyhedron
         ===========||=========|========|========|=========|============
         MOVING     ||         |        |        |         |
         -----------||---------|--------|--------|---------|------------
         Empty      || n/a     | n/a    | n/a    | n/a     | no
         -----------||---------|--------|--------|---------|------------
         Point      || n/a     | n/a    | n/a    | ok      | check
         -----------||---------|--------|--------|---------|------------
         Edge       || n/a     | n/a    | ok     | check   | check
         -----------||---------|--------|--------|---------|------------
         Polygon    || n/a     | invert | invert | check   | check
         -----------||---------|--------|--------|---------|------------
         Polyhedron || ok      | invert | invert | invert  | check

         n/a    - This case can never occur.
         ok     - This case is always allowed, unless the brush becomes invalid, i.e., not a polyhedron.
         no     - This case is always forbidden.
         invert - This case is handled by swapping the remaining and the moving fragments and inverting the delta. This takes us from a cell at (column, row) to the cell at (row, column).
         check  - Check whether any of the moved vertices would travel through the remaining fragment, or vice versa if inverted case. Also check whether the brush would become invalid, i.e., not a polyhedron.

         If `allowVertexRemoval` is true, vertices can be moved inside a remaining polyhedron.

         */
        Brush::CanMoveVerticesResult Brush::doCanMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, vm::vec3 delta, const bool allowVertexRemoval) const {
            // Should never occur, takes care of the first row.
            if (vertexPositions.empty() || vm::is_zero(delta, vm::C::almost_zero())) {
                return CanMoveVerticesResult::rejectVertexMove();
            }

            const auto vertexSet = std::set<vm::vec3>(std::begin(vertexPositions), std::end(vertexPositions));

            std::vector<vm::vec3> remainingPoints;
            remainingPoints.reserve(vertexCount());
            
            std::vector<vm::vec3> movingPoints;
            movingPoints.reserve(vertexCount());
            
            std::vector<vm::vec3> resultPoints;
            resultPoints.reserve(vertexCount());
            
            for (const auto* vertex : m_geometry->vertices()) {
                const auto& position = vertex->position();
                if (!vertexSet.count(position)) {
                    // the vertex is not moving
                    remainingPoints.push_back(position);
                    resultPoints.push_back(position);
                } else {
                    // the vertex is moving
                    movingPoints.push_back(position);
                    resultPoints.push_back(position + delta);
                }
            }

            BrushGeometry remaining(remainingPoints);
            BrushGeometry moving(movingPoints);
            BrushGeometry result(resultPoints);

            // Will the result go out of world bounds?
            if (!worldBounds.contains(result.bounds())) {
                return CanMoveVerticesResult::rejectVertexMove();
            }

            // Special case, takes care of the first column.
            if (moving.vertexCount() == vertexCount()) {
                return CanMoveVerticesResult::acceptVertexMove(std::move(result));
            }

            // Will vertices be removed?
            if (!allowVertexRemoval) {
                // All moving vertices must still be present in the result
                for (const auto& movingVertex : moving.vertexPositions()) {
                    if (!result.hasVertex(movingVertex + delta)) {
                        return CanMoveVerticesResult::rejectVertexMove();
                    }
                }
            }

            // Will the brush become invalid?
            if (!result.polyhedron()) {
                return CanMoveVerticesResult::rejectVertexMove();
            }

            // One of the remaining two ok cases?
            if ((moving.point() && remaining.polygon()) ||
                (moving.edge() && remaining.edge())) {
                return CanMoveVerticesResult::acceptVertexMove(std::move(result));
            }

            // Invert if necessary.
            if (remaining.point() || remaining.edge() || (remaining.polygon() && moving.polyhedron())) {
                using std::swap;
                swap(remaining, moving);
                delta = -delta;
            }

            // Now check if any of the moving vertices would travel through the remaining fragment and out the other side.
            for (const auto* vertex : moving.vertices()) {
                const auto& oldPos = vertex->position();
                const auto newPos = oldPos + delta;

                for (const auto* face : remaining.faces()) {
                    if (face->pointStatus(oldPos, vm::constants<FloatType>::point_status_epsilon()) == vm::plane_status::below &&
                        face->pointStatus(newPos, vm::constants<FloatType>::point_status_epsilon()) == vm::plane_status::above) {
                        const auto ray = vm::ray3(oldPos, normalize(newPos - oldPos));
                        const auto distance = face->intersectWithRay(ray, vm::side::back);
                        if (!vm::is_nan(distance)) {
                            return CanMoveVerticesResult::rejectVertexMove();
                        }
                    }
                }
            }

            return CanMoveVerticesResult::acceptVertexMove(std::move(result));
        }

        void Brush::doMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, const bool uvLock) {
            ensure(m_geometry != nullptr, "geometry is null");
            ensure(!vertexPositions.empty(), "no vertex positions");
            assert(canMoveVertices(worldBounds, vertexPositions, delta));

            std::vector<vm::vec3> newVertices;
            newVertices.reserve(vertexCount());
            
            for (const auto* vertex : m_geometry->vertices()) {
                const auto& position = vertex->position();
                if (kdl::vec_contains(vertexPositions, position)) {
                    newVertices.push_back(position + delta);
                } else {
                    newVertices.push_back(position);
                }
            }
            
            BrushGeometry newGeometry(newVertices);

            using VecMap = std::map<vm::vec3, vm::vec3>;
            VecMap vertexMapping;
            for (auto* oldVertex : m_geometry->vertices()) {
                const auto& oldPosition = oldVertex->position();
                const auto moved = kdl::vec_contains(vertexPositions, oldPosition);
                const auto newPosition = moved ? oldPosition + delta : oldPosition;
                const auto* newVertex = newGeometry.findClosestVertex(newPosition, vm::C::almost_zero());
                if (newVertex != nullptr) {
                    vertexMapping.insert(std::make_pair(oldPosition, newVertex->position()));
                }
            }

            const PolyhedronMatcher<BrushGeometry> matcher(*m_geometry, newGeometry, vertexMapping);
            doSetNewGeometry(worldBounds, matcher, newGeometry, uvLock);
        }

        std::tuple<bool, vm::mat4x4> Brush::findTransformForUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, BrushFaceGeometry* left, BrushFaceGeometry* right) {
            std::vector<vm::vec3> unmovedVerts;
            std::vector<std::pair<vm::vec3, vm::vec3>> movedVerts;

            matcher.visitMatchingVertexPairs(left, right, [&](BrushVertex* leftVertex, BrushVertex* rightVertex){
                const auto leftPosition = leftVertex->position();
                const auto rightPosition = rightVertex->position();

                if (vm::is_equal(leftPosition, rightPosition, vm::constants<FloatType>::almost_zero())) {
                    unmovedVerts.push_back(leftPosition);
                } else {
                    movedVerts.emplace_back(leftPosition, rightPosition);
                }
            });

            // If 3 or more are unmoving, give up.
            // (Picture a square with one corner being moved, we can't possibly lock the UV's of all 4 corners.)
            if (unmovedVerts.size() >= 3) {
                return std::make_tuple(false, vm::mat4x4());
            }

            std::vector<std::pair<vm::vec3, vm::vec3>> referenceVerts;

            // Use unmoving, then moving
            for (const auto& unmovedVert : unmovedVerts) {
                referenceVerts.emplace_back(unmovedVert, unmovedVert);
            }
            // TODO: When there are multiple choices of moving verts (unmovedVerts.size() + movedVerts.size() > 3)
            // we should sort them somehow. This can be seen if you select and move 3/5 verts of a pentagon;
            // which of the 3 moving verts currently gets UV lock is arbitrary.
            kdl::vec_append(referenceVerts, movedVerts);

            if (referenceVerts.size() < 3) {
                // Can't create a transform as there are not enough verts
                return std::make_tuple(false, vm::mat4x4());
            }

            const auto M = vm::points_transformation_matrix(
                    referenceVerts[0].first, referenceVerts[1].first, referenceVerts[2].first,
                    referenceVerts[0].second, referenceVerts[1].second, referenceVerts[2].second);

            if (!(M == M)) {
                // Transform contains nan
                return std::make_tuple(false, vm::mat4x4());
            }

            return std::make_tuple(true, M);
        }

        void Brush::applyUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, const BrushFace& leftFace, BrushFace& rightFace) {
            const auto [success, M] = findTransformForUVLock(matcher, leftFace.geometry(), rightFace.geometry());
            if (!success) {
                return;
            }

            // We want to re-set the texturing of `rightFace` using the texturing from M * leftFace.
            // We don't want to disturb the actual geometry of `rightFace` which is already finalized.
            // So the idea is, clone `leftFace`, transform it by M using texture lock, then copy the texture
            // settings from the transformed clone (which should have an identical plane to `rightFace` within
            // FP error) to `rightFace`.
            BrushFace leftClone = leftFace;

            try {
                leftClone.transform(M, true);

                auto snapshot = std::unique_ptr<TexCoordSystemSnapshot>(leftClone.takeTexCoordSystemSnapshot());
                rightFace.setAttributes(leftClone.attributes());
                if (snapshot) {
                    // Note, the wrap style doesn't matter because the source and destination faces should have the same plane
                    rightFace.copyTexCoordSystemFromFace(*snapshot, leftClone.attributes().takeSnapshot(),
                                                          leftClone.boundary(), WrapStyle::Rotation);
                }
                rightFace.resetTexCoordSystemCache();
            } catch (const GeometryException&) {
                // do nothing
            }
        }

        void Brush::doSetNewGeometry(const vm::bbox3& worldBounds, const PolyhedronMatcher<BrushGeometry>& matcher, const BrushGeometry& newGeometry, const bool uvLock) {
            std::vector<BrushFace> newFaces;
            newFaces.reserve(newGeometry.faces().size());
            matcher.processRightFaces([&](BrushFaceGeometry* left, BrushFaceGeometry* right){
                if (const auto leftFaceIndex = left->payload()) {
                    const BrushFace& leftFace = m_faces[*leftFaceIndex];
                    BrushFace& rightFace = newFaces.emplace_back(leftFace);

                    rightFace.setGeometry(right);
                    rightFace.updatePointsFromVertices();

                    if (uvLock) {
                        applyUVLock(matcher, leftFace, rightFace);
                    }
                }
            });

            m_faces = std::move(newFaces);
            updateGeometryFromFaces(worldBounds);
        }

        std::vector<Brush> Brush::subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const std::vector<const Brush*>& subtrahends) const {
            auto result = std::vector<BrushGeometry>{*m_geometry};

            for (auto* subtrahend : subtrahends) {
                auto nextResults = std::vector<BrushGeometry>();

                for (const BrushGeometry& fragment : result) {
                    auto subFragments = fragment.subtract(*subtrahend->m_geometry);

                    nextResults.reserve(nextResults.size() + subFragments.size());
                    for (auto& subFragment : subFragments) {
                        nextResults.push_back(std::move(subFragment));
                    }
                }

                result = std::move(nextResults);
            }

            std::vector<Brush> brushes;
            brushes.reserve(result.size());

            for (const auto& geometry : result) {
                try {
                    auto brush = createBrush(factory, worldBounds, defaultTextureName, geometry, subtrahends);
                    brushes.push_back(std::move(brush));
                } catch (const GeometryException&) {}
            }

            return brushes;
        }

        std::vector<Brush> Brush::subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const Brush& subtrahend) const {
            return subtract(factory, worldBounds, defaultTextureName, std::vector<const Brush*>{&subtrahend});
        }

        void Brush::intersect(const vm::bbox3& worldBounds, const Brush& brush) {
            for (const auto& face : brush.faces()) {
                m_faces.emplace_back(face);
            }

            updateGeometryFromFaces(worldBounds);
        }

        bool Brush::canTransform(const vm::mat4x4& transformation, const vm::bbox3& worldBounds) const {
            try {
                auto testBrush = Brush(*this);
                testBrush.transform(transformation, false, worldBounds);
                return true;
            } catch (GeometryException&) {
                return false;
            }
        }

        void Brush::transform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            for (auto& face : m_faces) {
                face.transform(transformation, lockTextures);
            }

            updateGeometryFromFaces(worldBounds);
        }

        bool Brush::contains(const vm::bbox3& bounds) const {
            if (!this->bounds().contains(bounds)) {
                return false;
            }

            for (const auto& vertex : bounds.vertices()) {
                if (!containsPoint(vertex)) {
                    return false;
                }
            }

            return true;
        }

        bool Brush::contains(const Brush& brush) const {
            return m_geometry->contains(*brush.m_geometry);
        }

        bool Brush::intersects(const vm::bbox3& bounds) const {
            return this->bounds().intersects(bounds);
        }

        bool Brush::intersects(const Brush& brush) const {
            return m_geometry->intersects(*brush.m_geometry);
        }

        Brush Brush::createBrush(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const BrushGeometry& geometry, const std::vector<const Brush*>& subtrahends) const {
            std::vector<BrushFace> faces;
            faces.reserve(geometry.faceCount());

            for (const auto* face : geometry.faces()) {
                const auto* h1 = face->boundary().front();
                const auto* h0 = h1->next();
                const auto* h2 = h0->next();

                const auto& p0 = h0->origin()->position();
                const auto& p1 = h1->origin()->position();
                const auto& p2 = h2->origin()->position();

                BrushFaceAttributes attribs(defaultTextureName);
                faces.push_back(factory.createFace(p0, p1, p2, attribs));
            }

            auto brush = Brush(worldBounds, std::move(faces));
            brush.cloneFaceAttributesFrom(*this);
            for (const auto* subtrahend : subtrahends) {
                brush.cloneInvertedFaceAttributesFrom(*subtrahend);
            }
            return brush;
        }

        void Brush::findIntegerPlanePoints(const vm::bbox3& worldBounds) {
            for (auto& face : m_faces) {
                face.findIntegerPlanePoints();
            }
            updateGeometryFromFaces(worldBounds);
        }

        bool Brush::checkFaceLinks() const {
            if (faceCount() != m_geometry->faceCount()) {
                return false;
            }
            
            const auto findFaceGeometry = [&](const BrushFaceGeometry* g) {
                for (const auto* fg : m_geometry->faces()) {
                    if (fg == g) {
                        return true;
                    }
                }
                return false;
            };
            
            for (const auto* faceGeometry : m_geometry->faces()) {
                if (const auto faceIndex = faceGeometry->payload()) {
                    if (*faceIndex >= m_faces.size()) {
                        return false;
                    }
                } else {
                    return false;
                }
            }
            
            std::set<const BrushFaceGeometry*> faceGeometries;
            for (const auto& face : m_faces) {
                const auto* faceGeometry = face.geometry();
                if (faceGeometry == nullptr) {
                    return false;
                }
                if (!findFaceGeometry(faceGeometry)) {
                    return false;
                }
                if (const auto faceIndex = faceGeometry->payload()) {
                    if (*faceIndex >= m_faces.size()) {
                        return false;
                    }
                    if (&m_faces[*faceIndex] != &face) {
                        return false;
                    }
                } else {
                    return false;
                }
                if (!faceGeometries.insert(faceGeometry).second) {
                    return false;
                }
            }
            
            return true;
        }
    }
}
