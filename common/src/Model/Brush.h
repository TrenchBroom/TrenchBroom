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

#pragma once

#include "FloatType.h"
#include "Macros.h"
#include "Model/BrushGeometry.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class ModelFactory;
        template <typename P> class PolyhedronMatcher;

        enum class BrushError;

        class Brush {
        private:
            class CopyCallback;
            
            /**
             * Epsilon value to use when finding a vertex after applying a vertex operation
             */
            constexpr static FloatType CloseVertexEpsilon = static_cast<FloatType>(0.01);
        public:
            using VertexList = BrushVertexList;
            using EdgeList = BrushEdgeList;
        private:
            std::vector<BrushFace> m_faces;
            std::unique_ptr<BrushGeometry> m_geometry;
        public:
            Brush();

            Brush(const Brush& other);
            Brush(Brush&& other) noexcept;
            Brush& operator=(Brush other) noexcept;

            friend void swap(Brush& lhs, Brush& rhs) noexcept;

            ~Brush();
            
            static kdl::result<Brush, BrushError> create(const vm::bbox3& worldBounds, std::vector<BrushFace> faces);
        private:
            Brush(std::vector<BrushFace> faces);

            kdl::result<void, BrushError> updateGeometryFromFaces(const vm::bbox3& worldBounds);
        public:
            const vm::bbox3& bounds() const;
        public: // face management:
            std::optional<size_t> findFace(const std::string& textureName) const;
            std::optional<size_t> findFace(const vm::vec3& normal) const;
            std::optional<size_t> findFace(const vm::plane3& boundary) const;
            std::optional<size_t> findFace(const vm::polygon3& vertices, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            std::optional<size_t> findFace(const std::vector<vm::polygon3>& candidates, FloatType epsilon = static_cast<FloatType>(0.0)) const;

            const BrushFace& face(size_t index) const;
            BrushFace& face(size_t index);
            size_t faceCount() const;
            const std::vector<BrushFace>& faces() const;
            std::vector<BrushFace>& faces();

            bool closed() const;
            bool fullySpecified() const;
        public: // clone face attributes from matching faces of other brushes
            void cloneFaceAttributesFrom(const Brush& brush);
            void cloneInvertedFaceAttributesFrom(const Brush& brush);
        public: // clipping
            kdl::result<Brush, BrushError> clip(const vm::bbox3& worldBounds, BrushFace face) const;
        public: // move face along normal
            /**
             * Translates a face by the given delta.
             *
             * The face is only translated if the resulting brush has the same number of faces as this brush. If the
             * resulting brush becomes invalid, an error is returned.
             *
             * @param worldBounds the world bounds
             * @param faceIndex the index of the face to translate
             * @param delta the vector by which to translate the face
             * @param lockTexture whether textures should be locked
             *
             * @return a result containing either the resulting brush or an error
             */
            kdl::result<Brush, BrushError> moveBoundary(const vm::bbox3& worldBounds, size_t faceIndex, const vm::vec3& delta, bool lockTexture) const;

            /**
             * Moves all faces by `delta` units along their normals; negative values shrink the brush. If the resulting
             * brush becomes invalid, an error is returned.
             *
             * @param worldBounds the world bounds
             * @param delta the distance by which to move the faces
             * @param lockTexture whether textures should be locked
             *
             * @return a result containing either the resulting brush or an error
             */
            kdl::result<Brush, BrushError> expand(const vm::bbox3& worldBounds, FloatType delta, bool lockTexture) const;
        public:
            // geometry access
            size_t vertexCount() const;
            const VertexList& vertices() const;
            const std::vector<vm::vec3> vertexPositions() const;

            vm::vec3 findClosestVertexPosition(const vm::vec3& position) const;
            std::vector<vm::vec3> findClosestVertexPositions(const std::vector<vm::vec3>& positions) const;
            std::vector<vm::segment3> findClosestEdgePositions(const std::vector<vm::segment3>& positions) const;
            std::vector<vm::polygon3> findClosestFacePositions(const std::vector<vm::polygon3>& positions) const;

            bool hasVertex(const vm::vec3& position, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdge(const vm::segment3& edge, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::polygon3& face, FloatType epsilon = static_cast<FloatType>(0.0)) const;

            size_t edgeCount() const;
            const EdgeList& edges() const;
            bool containsPoint(const vm::vec3& point) const;

            std::vector<const BrushFace*> incidentFaces(const BrushVertex* vertex) const;

            // vertex operations
            bool canMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertices, const vm::vec3& delta) const;
            kdl::result<Brush, BrushError> moveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, bool uvLock = false) const;

            bool canAddVertex(const vm::bbox3& worldBounds, const vm::vec3& position) const;
            kdl::result<Brush, BrushError> addVertex(const vm::bbox3& worldBounds, const vm::vec3& position) const;

            bool canRemoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions) const;
            kdl::result<Brush, BrushError> removeVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions) const;

            bool canSnapVertices(const vm::bbox3& worldBounds, FloatType snapTo) const;
            kdl::result<Brush, BrushError> snapVertices(const vm::bbox3& worldBounds, FloatType snapTo, bool uvLock = false) const;

            // edge operations
            bool canMoveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta) const;
            kdl::result<Brush, BrushError> moveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta, bool uvLock = false) const;

            // face operations
            bool canMoveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta) const;
            kdl::result<Brush, BrushError> moveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta, bool uvLock = false) const;
        private:
            struct CanMoveVerticesResult {
            public:
                bool success;
                std::unique_ptr<BrushGeometry> geometry;
            private:
                CanMoveVerticesResult(bool s, BrushGeometry&& g);
            public:
                static CanMoveVerticesResult rejectVertexMove();
                static CanMoveVerticesResult acceptVertexMove(BrushGeometry&& result);
            };

            CanMoveVerticesResult doCanMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, vm::vec3 delta, bool allowVertexRemoval) const;
            kdl::result<Brush, BrushError> doMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, bool lockTexture) const;
            /**
             * Tries to find 3 vertices in `left` and `right` that are related according to the PolyhedronMatcher, and
             * generates an affine transform for them which can then be used to implement UV lock.
             *
             * @param matcher a polyhedron matcher which is used to identify related vertices
             * @param left the face of the left polyhedron
             * @param right the face of the right polyhedron
             * @return {true, transform} if a transform could be found, otherwise {false, unspecified}
             */
            static std::tuple<bool, vm::mat4x4> findTransformForUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, BrushFaceGeometry* left, BrushFaceGeometry* right);
            
            /**
             * Helper function to apply UV lock to the face `right`.
             *
             * It's assumed that `left` and `right` have already been identified as "matching" faces for a vertex move
             * where `left` is a face from the polyhedron before vertex manipulation, and `right` is from the newly
             * modified brush.
             *
             * This function tries to pick 3 vertices from `left` and `right` to generate a transform
             * (using findTransformForUVLock), and updates the texturing of `right` using that transform applied to `left`.
             * If it can't perform UV lock, `right` remains unmodified.
             *
             * This is only meant to be called in the matcher callback in Brush::createBrushWithNewGeometry
             *
             * @param matcher a polyhedron matcher which is used to identify related vertices
             * @param leftFace the face of the left polyhedron
             * @param rightFace the face of the right polyhedron
             */
            static void applyUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, const BrushFace& leftFace, BrushFace& rightFace);
            kdl::result<Brush, BrushError> createBrushWithNewGeometry(const vm::bbox3& worldBounds, const PolyhedronMatcher<BrushGeometry>& matcher, const BrushGeometry& newGeometry, bool uvLock = false) const;
        public:
            // CSG operations
            /**
             * Subtracts the given subtrahends from `this`, returning the result but without modifying `this`.
             *
             * @param subtrahends brushes to subtract from `this`. The passed-in brushes are not modified.
             * @return the subtraction result
             */
            kdl::result<std::vector<Brush>, BrushError> subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const std::vector<const Brush*>& subtrahends) const;
            kdl::result<std::vector<Brush>, BrushError> subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const Brush& subtrahend) const;

            /**
             * Intersects this brush with the given brush and returns the resulting brush.
             *
             * If the resulting brush is invalid, an error is returned.
             *
             * @param worldBounds the world bounds
             * @param brush the brush to intersect this brush with
             * @return the intersection brush or an error if the operation fails
             */
            kdl::result<Brush, BrushError> intersect(const vm::bbox3& worldBounds, const Brush& brush) const;

            /**
             * Applies the given transformation to this brush and returns the resulting brush.
             *
             * If the resulting brush is invalid, an error is returned.
             *
             * @param worldBounds the world bounds
             * @param transformation the transformation to apply
             * @param lockTextures whether textures should be locked
             * @return the transformed brush or an error if the operation fails
             */
            kdl::result<Brush, BrushError> transform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) const;
        public:
            bool contains(const vm::bbox3& bounds) const;
            bool contains(const Brush& brush) const;
            bool intersects(const vm::bbox3& bounds) const;
            bool intersects(const Brush& brush) const;
        private:
            /**
             * Final step of CSG subtraction; takes the geometry that is the result of the subtraction, and turns it
             * into a Brush by copying texturing from `this` (for un-clipped faces) or the brushes in `subtrahends`
             * (for clipped faces).
             *
             * @param factory the model factory
             * @param worldBounds the world bounds
             * @param defaultTextureName default texture name
             * @param geometry the geometry for the newly created brush
             * @param subtrahends used as a source of texture alignment only
             * @return the newly created brush
             */
            kdl::result<Brush, BrushError> createBrush(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const BrushGeometry& geometry, const std::vector<const Brush*>& subtrahends) const;
        public: // texture format conversion
            Brush convertToParaxial() const;
            Brush convertToParallel() const;
        private:
            bool checkFaceLinks() const;
        };

        bool operator==(const Brush& lhs, const Brush& rhs);
        bool operator!=(const Brush& lhs, const Brush& rhs);
    }
}

#endif /* defined(TrenchBroom_Brush) */
