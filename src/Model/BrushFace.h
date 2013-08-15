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

#ifndef __TrenchBroom__Face__
#define __TrenchBroom__Face__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Allocator.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Model/ModelTypes.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;
        class BrushFaceGeometry;
        
        class BrushFace {
        public:
            /*
             * The order of points, when looking from outside the face:
             *
             * 0-----------1
             * |
             * |
             * |
             * |
             * 2
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
            String m_textureName;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            size_t m_surfaceContents;
            size_t m_surfaceFlags;
            float m_surfaceValue;
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            
            Assets::FaceTexture* m_texture;
            BrushFaceGeometry* m_side;
            
            mutable Vertex::List m_cachedVertices;
            mutable bool m_vertexCacheValid;
        protected:
            BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName);
        public:
            virtual ~BrushFace();
            
            Brush* parent() const;
            void setParent(Brush* parent);
            
            const BrushFace::Points& points() const;
            bool arePointsOnPlane(const Plane3& plane) const;
            
            const String& textureName() const;
            Assets::FaceTexture* texture() const;
            const Plane3& boundary() const;
            float xOffset() const;
            float yOffset() const;
            float rotation() const;
            float xScale() const;
            float yScale() const;
            size_t surfaceContents() const;
            size_t surfaceFlags() const;
            float surfaceValue() const;
            
            void setTexture(Assets::FaceTexture* texture);
            void setXOffset(const float xOffset);
            void setYOffset(const float yOffset);
            void setRotation(const float rotation);
            void setXScale(const float xScale);
            void setYScale(const float yScale);
            void setSurfaceContents(const size_t surfaceContents);
            void setSurfaceFlags(const size_t surfaceFlags);
            void setSurfaceValue(const float surfaceValue);
            void setFilePosition(const size_t lineNumber, const size_t lineCount);
            void setSide(BrushFaceGeometry* side);
            
            bool selected() const;
            void select();
            void deselect();

            void addToMesh(Mesh& mesh) const;
            FloatType intersectWithRay(const Ray3& ray) const;
        private:
            void setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2);
            void validateVertexCache() const;

            virtual void updateTextureCoordinateSystem(const Vec3& normal, const float rotation) = 0;
            virtual Vec2f textureCoordinates(const Vec3& point, const float xOffset, const float yOffset, const float xScale, const float yScale, const size_t textureWidth, const size_t textureHeight) const = 0;

            BrushFace(const BrushFace& other);
            BrushFace& operator=(const BrushFace& other);
        };
        
        template <class TexCoordSystem>
        class ConfigurableBrushFace : public BrushFace, public Allocator<ConfigurableBrushFace<TexCoordSystem> > {
        private:
            TexCoordSystem m_coordSystem;
        public:
            ConfigurableBrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = NoTextureName) :
            BrushFace(point0, point1, point2, textureName) {}

            ConfigurableBrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const Vec3& textureXAxis, const Vec3& textureYAxis, const Vec3& normal, const float rotation, const String& textureName = NoTextureName) :
            BrushFace(point0, point1, point2, textureName),
            m_coordSystem(textureXAxis, textureYAxis, normal, rotation) {}
        private:
            inline void updateTextureCoordinateSystem(const Vec3& normal, const float rotation) {
                m_coordSystem.update(normal, rotation);
            }
            
            inline Vec2f textureCoordinates(const Vec3& point, const float xOffset, const float yOffset, const float xScale, const float yScale, const size_t textureWidth, const size_t textureHeight) const {
                const float safeXScale = xScale == 0.0f ? 1.0f : xScale;
                const float safeYScale = yScale == 0.0f ? 1.0f : yScale;
                const float x = static_cast<float>((point.dot(m_coordSystem.xAxis() * safeXScale) + xOffset) / textureWidth);
                const float y = static_cast<float>((point.dot(m_coordSystem.yAxis() * safeYScale) + yOffset) / textureHeight);
                return Vec2f(x, y);
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */
