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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Bsp29Model__
#define __TrenchBroom__Bsp29Model__

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/EntityModel.h"
#include "Assets/AssetTypes.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class MeshRenderer;
        class Vbo;
    }
    
    namespace Assets {
        class Bsp29Model : public EntityModel {
        public:
            class Face {
            private:
                Vec3f::List m_vertices;
                Vec2f::List m_texCoords;
                Texture* m_texture;
            public:
                Face(Texture* texture);
                void addVertex(const Vec3f& vertex, const Vec2f& texCoord);
                
                Texture* texture() const;
                Renderer::VertexSpecs::P3T2::Vertex::List vertices() const;
                const Vec3f::List& vertexPositions() const;
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
            Renderer::MeshRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Bsp29Model__) */
