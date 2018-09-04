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
            typedef vec3 Points[3];
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
            plane3 m_boundary;
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
            BrushFace(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs, TexCoordSystem* texCoordSystem);
            
            static BrushFace* createParaxial(const vec3& point0, const vec3& point1, const vec3& point2, const String& textureName = "");
            static BrushFace* createParallel(const vec3& point0, const vec3& point1, const vec3& point2, const String& textureName = "");
            
            static void sortFaces(BrushFaceList& faces);
            
            virtual ~BrushFace();
            
            BrushFace* clone() const;
            
            BrushFaceSnapshot* takeSnapshot();
            TexCoordSystemSnapshot* takeTexCoordSystemSnapshot() const;
            void restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot* coordSystemSnapshot);
            void copyTexCoordSystemFromFace(const TexCoordSystemSnapshot* coordSystemSnapshot, const BrushFaceAttributes& attribs, const plane3& sourceFacePlane, const WrapStyle wrapStyle);

            Brush* brush() const;
            void setBrush(Brush* brush);
            
            const BrushFace::Points& points() const;
            bool arePointsOnPlane(const plane3& plane) const;
            const plane3& boundary() const;
            const vec3& normal() const;
            vec3 center() const;
            vec3 boundsCenter() const;
            FloatType area(Math::Axis::Type axis) const;
            
            const BrushFaceAttributes& attribs() const;
            void setAttribs(const BrushFaceAttributes& attribs);

            void resetTexCoordSystemCache();
            
            const String& textureName() const;
            Assets::Texture* texture() const;
            vec2f textureSize() const;
            
            const vec2f& offset() const;
            float xOffset() const;
            float yOffset() const;
            vec2f modOffset(const vec2f& offset) const;

            const vec2f& scale() const;
            float xScale() const;
            float yScale() const;

            float rotation() const;

            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            bool hasSurfaceAttributes() const;

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

            vec3 textureXAxis() const;
            vec3 textureYAxis() const;
            void resetTextureAxes();
            
            void moveTexture(const vec3& up, const vec3& right, const vec2f& offset);
            void rotateTexture(float angle);
            void shearTexture(const vec2f& factors);
            
            void transform(const mat4x4& transform, const bool lockTexture);
            void invert();

            void updatePointsFromVertices();
            void snapPlanePointsToInteger();
            void findIntegerPlanePoints();
            
            mat4x4 projectToBoundaryMatrix() const;
            mat4x4 toTexCoordSystemMatrix(const vec2f& offset, const vec2f& scale, bool project) const;
            mat4x4 fromTexCoordSystemMatrix(const vec2f& offset, const vec2f& scale, bool project) const;
            float measureTextureAngle(const vec2f& center, const vec2f& point) const;
            
            size_t vertexCount() const;
            EdgeList edges() const;
            VertexList vertices() const;
            vec3::List vertexPositions() const;
            
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

            vec2f textureCoords(const vec3& point) const;

            bool containsPoint(const vec3& point) const;
            FloatType intersectWithRay(const ray3& ray) const;
            
            void printPoints() const;
        private:
            void setPoints(const vec3& point0, const vec3& point1, const vec3& point2);
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
