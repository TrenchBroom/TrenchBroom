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

#ifndef __TrenchBroom__Face__
#define __TrenchBroom__Face__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Allocator.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Model/ModelTypes.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/TexCoordSystem.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;
        class BrushFaceGeometry;
        
        class BrushFaceAttribs {
        private:
            String m_textureName;
            Assets::Texture* m_texture;
            
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            
            int m_surfaceContents;
            int m_surfaceFlags;
            float m_surfaceValue;
        public:
            BrushFaceAttribs(const String& textureName);
            
            const String& textureName() const;
            Assets::Texture* texture() const;

            float xOffset() const;
            float yOffset() const;
            float rotation() const;
            float xScale() const;
            float yScale() const;
            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            
            void setTexture(Assets::Texture* texture);
            void setXOffset(float xOffset);
            void setYOffset(float yOffset);
            void setRotation(float rotation);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void setSurfaceContents(int surfaceContents);
            void setSurfaceFlags(int surfaceFlags);
            void setSurfaceValue(float surfaceValue);
        };
        
        class BrushFaceSnapshot {
        private:
            BrushFace* m_face;
            BrushFaceAttribs m_attribs;
        public:
            BrushFaceSnapshot(BrushFace& face);
            void restore();
        };
        
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
            
            typedef Renderer::VertexSpecs::P3NT2 VertexSpec;
            typedef VertexSpec::Vertex Vertex;
            typedef Renderer::Mesh<const Assets::Texture*, VertexSpec> Mesh;
            static const String NoTextureName;
        private:
            Brush* m_parent;
            BrushFace::Points m_points;
            Plane3 m_boundary;
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            
            TexCoordSystem* m_texCoordSystem;
            BrushFaceGeometry* m_side;
            mutable Vertex::List m_cachedVertices;
            mutable bool m_vertexCacheValid;
        protected:
            BrushFaceAttribs m_attribs;
        public:
            BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName, TexCoordSystem* texCoordSystem);
            
            static BrushFace* createParaxial(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = "");
            static BrushFace* createParallel(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = "");
            
            virtual ~BrushFace();
            
            BrushFace* clone() const;
            
            BrushFaceSnapshot takeSnapshot();

            Brush* parent() const;
            void setParent(Brush* parent);
            
            const BrushFace::Points& points() const;
            bool arePointsOnPlane(const Plane3& plane) const;
            const Plane3& boundary() const;
            Vec3 center() const;

            const BrushFaceAttribs& attribs() const;
            void setAttribs(const BrushFaceAttribs& attribs);
            
            const String& textureName() const;
            Assets::Texture* texture() const;
            float xOffset() const;
            float yOffset() const;
            float rotation() const;
            float xScale() const;
            float yScale() const;
            int surfaceContents() const;
            int surfaceFlags() const;
            float surfaceValue() const;
            
            void setTexture(Assets::Texture* texture);
            void setXOffset(float xOffset);
            void setYOffset(float yOffset);
            void setRotation(float rotation);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void setSurfaceContents(int surfaceContents);
            void setSurfaceFlags(int surfaceFlags);
            void setSurfaceValue(float surfaceValue);
            void setAttributes(const BrushFace& other);

            void moveTexture(const Vec3& up, const Vec3& right, Math::Direction direction, float distance);
            void rotateTexture(float angle);
            
            void transform(const Mat4x4& transform, const bool lockTexture);
            void invert();

            void updatePointsFromVertices();
            void snapPlanePointsToInteger();
            void findIntegerPlanePoints();
            
            Mat4x4 toTexCoordSystemMatrix(const Vec2f& offset = Vec2f::Null, const Vec2f& scale = Vec2f(1.0f, 1.0f)) const;
            Mat4x4 fromTexCoordSystemMatrix(const Vec2f& offset = Vec2f::Null, const Vec2f& scale = Vec2f(1.0f, 1.0f)) const;
            
            const BrushEdgeList& edges() const;
            const BrushVertexList& vertices() const;
            
            BrushFaceGeometry* side() const;
            void setSide(BrushFaceGeometry* side);
            
            void setFilePosition(const size_t lineNumber, const size_t lineCount);
            
            bool selected() const;
            void select();
            void deselect();

            void addToMesh(Mesh& mesh) const;
            Vec2f textureCoords(const Vec3& point) const;
            
            FloatType intersectWithRay(const Ray3& ray) const;
            
            void invalidate();
        private:
            void setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2);
            void correctPoints();
            void validateVertexCache() const;
            void invalidateVertexCache();
            
            BrushFace(const BrushFace& other);
            BrushFace& operator=(const BrushFace& other);
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */
