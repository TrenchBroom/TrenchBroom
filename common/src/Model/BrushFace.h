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

#ifndef TrenchBroom_Face
#define TrenchBroom_Face

#include "FloatType.h"
#include "Macros.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushGeometry.h"
#include "Model/Tag.h" // BrushFace inherits from Taggable

#include <kdl/transform_range.h>

#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/util.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
        class TextureManager;
    }

    namespace Model {
        class BrushNode;
        class BrushFaceSnapshot;
        class TexCoordSystem;
        class TexCoordSystemSnapshot;
        enum class WrapStyle;

        class BrushFace : public Taggable {
        public:
            /*
             * The order of points, when looking from outside the face:
             *
             * 1
             * |
             * |
             * |
             * |
             * 0-----------2
             */
            using Points = vm::vec3[3];
        private:
            /**
             * For use in VertexList transformation below.
             */
            struct TransformHalfEdgeToVertex {
                const BrushVertex* operator()(const BrushHalfEdge* halfEdge) const;
            };

            /**
             * For use in EdgeList transformation below.
             */
            struct TransformHalfEdgeToEdge {
                const BrushEdge* operator()(const BrushHalfEdge* halfEdge) const;
            };
        public:
            using VertexList = kdl::transform_adapter<BrushHalfEdgeList, TransformHalfEdgeToVertex>;
            using EdgeList = kdl::transform_adapter<BrushHalfEdgeList, TransformHalfEdgeToEdge>;
        private:
            BrushNode* m_brush;
            BrushFace::Points m_points;
            vm::plane3 m_boundary;
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;

            std::unique_ptr<TexCoordSystem> m_texCoordSystem;
            BrushFaceGeometry* m_geometry;

            // brush renderer
            mutable bool m_markedToRenderFace;
        protected:
            BrushFaceAttributes m_attribs;
        public:
            BrushFace(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const BrushFaceAttributes& attribs, std::unique_ptr<TexCoordSystem> texCoordSystem);

            static BrushFace* createParaxial(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const std::string& textureName = "");
            static BrushFace* createParallel(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const std::string& textureName = "");

            static void sortFaces(std::vector<BrushFace*>& faces);

            virtual ~BrushFace() override;

            BrushFace* clone() const;

            BrushFaceSnapshot* takeSnapshot();
            std::unique_ptr<TexCoordSystemSnapshot> takeTexCoordSystemSnapshot() const;
            void restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot& coordSystemSnapshot);
            void copyTexCoordSystemFromFace(const TexCoordSystemSnapshot& coordSystemSnapshot, const BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, WrapStyle wrapStyle);

            BrushNode* brush() const;
            void setBrush(BrushNode* brush);

            const BrushFace::Points& points() const;
            bool arePointsOnPlane(const vm::plane3& plane) const;
            const vm::plane3& boundary() const;
            const vm::vec3& normal() const;
            vm::vec3 center() const;
            vm::vec3 boundsCenter() const;
            FloatType area(vm::axis::type axis) const;

            const BrushFaceAttributes& attribs() const;
            void setAttribs(const BrushFaceAttributes& attribs);

            void resetTexCoordSystemCache();

            const std::string& textureName() const;
            Assets::Texture* texture() const;
            vm::vec2f textureSize() const;

            const vm::vec2f& offset() const;
            float xOffset() const;
            float yOffset() const;
            vm::vec2f modOffset(const vm::vec2f& offset) const;

            const vm::vec2f& scale() const;
            float xScale() const;
            float yScale() const;

            float rotation() const;

            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            bool hasSurfaceAttributes() const;

            bool hasColor() const;
            const Color& color() const;
            bool setColor(const Color& color);

            void updateTexture(Assets::TextureManager& textureManager);
            bool setTexture(Assets::Texture* texture);
            bool unsetTexture();

            bool setXOffset(float xOffset);
            bool setYOffset(float yOffset);
            bool setXScale(float xScale);
            bool setYScale(float yScale);
            bool setRotation(float rotation);
            bool setSurfaceContents(int surfaceContents);
            bool setSurfaceFlags(int surfaceFlags);
            bool setSurfaceValue(float surfaceValue);
            bool setAttributes(const BrushFace* other);

            vm::vec3 textureXAxis() const;
            vm::vec3 textureYAxis() const;
            void resetTextureAxes();

            void moveTexture(const vm::vec3& up, const vm::vec3& right, const vm::vec2f& offset);
            void rotateTexture(float angle);
            void shearTexture(const vm::vec2f& factors);

            void transform(const vm::mat4x4& transform, bool lockTexture);
            void invert();

            void updatePointsFromVertices();
            void snapPlanePointsToInteger();
            void findIntegerPlanePoints();

            vm::mat4x4 projectToBoundaryMatrix() const;
            vm::mat4x4 toTexCoordSystemMatrix(const vm::vec2f& offset, const vm::vec2f& scale, bool project) const;
            vm::mat4x4 fromTexCoordSystemMatrix(const vm::vec2f& offset, const vm::vec2f& scale, bool project) const;
            float measureTextureAngle(const vm::vec2f& center, const vm::vec2f& point) const;

            size_t vertexCount() const;
            EdgeList edges() const;
            VertexList vertices() const;
            std::vector<vm::vec3> vertexPositions() const;

            bool hasVertices(const vm::polygon3& vertices, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            vm::polygon3 polygon() const;
        public:
            BrushFaceGeometry* geometry() const;
            void setGeometry(BrushFaceGeometry* geometry);
            void invalidate();

            size_t lineNumber() const;
            void setFilePosition(size_t lineNumber, size_t lineCount);

            bool selected() const;
            void select();
            void deselect();

            vm::vec2f textureCoords(const vm::vec3& point) const;

            FloatType intersectWithRay(const vm::ray3& ray) const;
        private:
            void setPoints(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2);
            void correctPoints();

            void updateBrush();

            // renderer cache
            void invalidateVertexCache();
        public: // brush renderer
            /**
             * This is used to cache results of evaluating the BrushRenderer Filter.
             * It's only valid within a call to `BrushRenderer::validateBrush`.
             *
             * @param marked    whether the face is going to be rendered.
             */
            void setMarked(bool marked) const;
            bool isMarked() const;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(BrushFace)
        };
    }
}

#endif /* defined(TrenchBroom_Face) */
