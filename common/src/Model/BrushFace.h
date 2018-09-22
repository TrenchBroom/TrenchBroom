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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Allocator.h"
#include "ProjectingSequence.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushGeometry.h"
#include "Model/ModelTypes.h"
#include "Model/TexCoordSystem.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureManager;
    }
    
    namespace Renderer {
        class IndexRangeMap;
        class TexturedIndexArrayBuilder;
    }
    
    namespace Model {
        class Brush;
        class BrushFaceSnapshot;
        
        class BrushFace {
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
            typedef Vec3 Points[3];
        public:
            static const String NoTextureName;
        private:
            struct ProjectToVertex : public ProjectingSequenceProjector<BrushHalfEdge*, BrushVertex*> {
                static Type project(BrushHalfEdge* halfEdge);
            };
            
            struct ProjectToEdge : public ProjectingSequenceProjector<BrushHalfEdge*, BrushEdge*> {
                static Type project(BrushHalfEdge* halfEdge);
            };
        public:
            typedef ConstProjectingSequence<BrushHalfEdgeList, ProjectToVertex> VertexList;
            typedef ConstProjectingSequence<BrushHalfEdgeList, ProjectToEdge> EdgeList;
        private:
            Brush* m_brush;
            BrushFace::Points m_points;
            Plane3 m_boundary;
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            
            TexCoordSystem* m_texCoordSystem;
            BrushFaceGeometry* m_geometry;
            
            // brush renderer
            mutable bool m_markedToRenderFace;
        protected:
            BrushFaceAttributes m_attribs;
        public:
            BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs, TexCoordSystem* texCoordSystem);
            
            static BrushFace* createParaxial(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = "");
            static BrushFace* createParallel(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = "");
            
            static void sortFaces(BrushFaceList& faces);
            
            virtual ~BrushFace();
            
            BrushFace* clone() const;
            
            BrushFaceSnapshot* takeSnapshot();
            TexCoordSystemSnapshot* takeTexCoordSystemSnapshot() const;
            void restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot* coordSystemSnapshot);
            void copyTexCoordSystemFromFace(const TexCoordSystemSnapshot* coordSystemSnapshot, const BrushFaceAttributes& attribs, const Plane3& sourceFacePlane, const WrapStyle wrapStyle);

            Brush* brush() const;
            void setBrush(Brush* brush);
            
            const BrushFace::Points& points() const;
            bool arePointsOnPlane(const Plane3& plane) const;
            const Plane3& boundary() const;
            const Vec3& normal() const;
            Vec3 center() const;
            Vec3 boundsCenter() const;
            FloatType area(Math::Axis::Type axis) const;
            
            const BrushFaceAttributes& attribs() const;
            void setAttribs(const BrushFaceAttributes& attribs);

            void resetTexCoordSystemCache();
            
            const String& textureName() const;
            Assets::Texture* texture() const;
            Vec2f textureSize() const;
            
            const Vec2f& offset() const;
            float xOffset() const;
            float yOffset() const;
            Vec2f modOffset(const Vec2f& offset) const;

            const Vec2f& scale() const;
            float xScale() const;
            float yScale() const;

            float rotation() const;

            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            bool hasSurfaceAttributes() const;

            bool hasColor() const;
            const Color& color() const;
            void setColor(const Color& color);

            void updateTexture(Assets::TextureManager* textureManager);
            void setTexture(Assets::Texture* texture);
            void unsetTexture();
            
            void setXOffset(float xOffset);
            void setYOffset(float yOffset);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void setRotation(float rotation);
            void setSurfaceContents(int surfaceContents);
            void setSurfaceFlags(int surfaceFlags);
            void setSurfaceValue(float surfaceValue);
            void setAttributes(const BrushFace* other);

            Vec3 textureXAxis() const;
            Vec3 textureYAxis() const;
            void resetTextureAxes();
            
            void moveTexture(const Vec3& up, const Vec3& right, const Vec2f& offset);
            void rotateTexture(float angle);
            void shearTexture(const Vec2f& factors);
            
            void transform(const Mat4x4& transform, const bool lockTexture);
            void invert();

            void updatePointsFromVertices();
            void snapPlanePointsToInteger();
            void findIntegerPlanePoints();
            
            Mat4x4 projectToBoundaryMatrix() const;
            Mat4x4 toTexCoordSystemMatrix(const Vec2f& offset, const Vec2f& scale, bool project) const;
            Mat4x4 fromTexCoordSystemMatrix(const Vec2f& offset, const Vec2f& scale, bool project) const;
            float measureTextureAngle(const Vec2f& center, const Vec2f& point) const;
            
            size_t vertexCount() const;
            EdgeList edges() const;
            VertexList vertices() const;
            
            bool hasVertices(const Polygon3& vertices, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            Polygon3 polygon() const;
        public:
            BrushFaceGeometry* geometry() const;
            void setGeometry(BrushFaceGeometry* geometry);
            void invalidate();
            
            void setFilePosition(const size_t lineNumber, const size_t lineCount);
            
            bool selected() const;
            void select();
            void deselect();

            Vec2f textureCoords(const Vec3& point) const;

            bool containsPoint(const Vec3& point) const;
            FloatType intersectWithRay(const Ray3& ray) const;
            
            void printPoints() const;
        private:
            void setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2);
            void correctPoints();
            
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
        private:
            BrushFace(const BrushFace& other);
            BrushFace& operator=(const BrushFace& other);
        };
    }
}

#endif /* defined(TrenchBroom_Face) */
