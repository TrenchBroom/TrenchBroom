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

#ifndef TrenchBroom_Bsp29Model
#define TrenchBroom_Bsp29Model

#include "StringUtils.h"
#include "Assets/EntityModel.h"
#include "Assets/AssetTypes.h"
#include "Renderer/VertexSpec.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Bsp29Model : public EntityModel {
        public:
            class Face {
            public:
                using Vertex = Renderer::VertexSpecs::P3T2::Vertex;
                using VertexList = Vertex::List;
            private:
                Texture* m_texture;
                VertexList m_vertices;
            public:
                Face(Texture* texture, size_t vertexCount);
                void addVertex(const vm::vec3f& vertex, const vm::vec2f& texCoord);
                
                Texture* texture() const;
                const VertexList& vertices() const;
            };
            typedef std::vector<Face> FaceList;
        private:
            struct SubModel {
                FaceList faces;
                vm::bbox3f bounds;
                SubModel(const FaceList& i_faces, const vm::bbox3f& i_bounds);
                
                vm::bbox3f transformedBounds(const vm::mat4x4f& transformation) const;
            };

            typedef std::vector<SubModel> SubModelList;
            String m_name;
            SubModelList m_subModels;
            TextureCollection* m_textureCollection;
        public:
            Bsp29Model(const String& name, TextureCollection* textureCollection);
            ~Bsp29Model() override;
            
            void addModel(const FaceList& faces, const vm::bbox3f& bounds);
        private:
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const override;
            vm::bbox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const override;
            vm::bbox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const vm::mat4x4f& transformation) const override;
            void doPrepare(int minFilter, int magFilter) override;
            void doSetTextureMode(int minFilter, int magFilter) override;
        };
    }
}

#endif /* defined(TrenchBroom_Bsp29Model) */
