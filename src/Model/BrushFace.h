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
        
        struct TextureCoordinateSystem {
            Vec3 xAxis;
            Vec3 yAxis;
            
            Vec2f textureCoordinates(const Vec3& point, const float xOffset, const float yOffset, const float xScale, const float yScale, const size_t width, const size_t height) const;
        };
        
        class BrushFace : public Allocator<BrushFace> {
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
            typedef Renderer::Mesh<Assets::FaceTexture*, VertexSpec> Mesh;
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
            size_t m_lineNumber;
            size_t m_lineCount;
            bool m_selected;
            
            Assets::FaceTexture* m_texture;
            BrushFaceGeometry* m_side;
            
            TextureCoordinateSystem m_textureCoordSystem;
            
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
            
            void setTexture(Assets::FaceTexture* texture);
            void setXOffset(const float xOffset);
            void setYOffset(const float yOffset);
            void setRotation(const float rotation);
            void setXScale(const float xScale);
            void setYScale(const float yScale);
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

            virtual TextureCoordinateSystem textureCoordinateSystem(const Vec3& normal, const float rotation) = 0;

            BrushFace(const BrushFace& other);
            BrushFace& operator=(const BrushFace& other);
        };
        
        template <class TexCoordPolicy>
        class ConfigurableBrushFace : public BrushFace {
        public:
            ConfigurableBrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName = NoTextureName) :
            BrushFace(point0, point1, point2, textureName) {}
        private:
            inline TextureCoordinateSystem textureCoordinateSystem(const Vec3& normal, const float rotation) {
                return TexCoordPolicy::textureCoordinateSystem(normal, rotation);
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */
