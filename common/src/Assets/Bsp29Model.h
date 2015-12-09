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

#ifndef TrenchBroom_Bsp29Model
#define TrenchBroom_Bsp29Model

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/EntityModel.h"
#include "Assets/AssetTypes.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Bsp29Model : public EntityModel {
        public:
            class Face {
            public:
                typedef Renderer::VertexSpecs::P3T2::Vertex Vertex;
                typedef Vertex::List VertexList;
            private:
                Texture* m_texture;
                VertexList m_vertices;
            public:
                Face(Texture* texture, size_t vertexCount);
                void addVertex(const Vec3f& vertex, const Vec2f& texCoord);
                
                Texture* texture() const;
                const VertexList& vertices() const;
            };
            typedef std::vector<Face> FaceList;
        private:
            struct SubModel {
                FaceList faces;
                BBox3f bounds;
                SubModel(const FaceList& i_faces, const BBox3f& i_bounds);
                
                BBox3f transformedBounds(const Mat4x4f& transformation) const;
            };

            typedef std::vector<SubModel> SubModelList;
            String m_name;
            SubModelList m_subModels;
            TextureCollection* m_textureCollection;
        public:
            Bsp29Model(const String& name, TextureCollection* textureCollection);
            ~Bsp29Model();
            
            void addModel(const FaceList& faces, const BBox3f& bounds);
        private:
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
            void doPrepare(int minFilter, int magFilter);
            void doSetTextureMode(int minFilter, int magFilter);
        };
    }
}

#endif /* defined(TrenchBroom_Bsp29Model) */
