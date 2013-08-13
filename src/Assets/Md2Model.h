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

#ifndef __TrenchBroom__Md2Model__
#define __TrenchBroom__Md2Model__

#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "StringUtils.h"
#include "VecMath.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
    }
    
    namespace Assets {
        class Md2Model : public EntityModel {
        public:
            typedef Renderer::VertexSpecs::P3NT2 VertexSpec;
            typedef VertexSpec::Vertex Vertex;
            typedef Renderer::Mesh<Assets::AutoTexture*, VertexSpec> Mesh;
        private:
            struct Frame {
                Mesh::TriangleSeries triangleFans;
                Mesh::TriangleSeries triangleStrips;
                
                Frame(const Mesh::TriangleSeries& i_triangleFans, const Mesh::TriangleSeries& i_triangleStrips);
            };
            typedef std::vector<Frame> FrameList;
            
            String m_name;
            Assets::TextureList m_skins;
            FrameList m_frames;
        public:
            Md2Model(const String& name);
            ~Md2Model();
            
            void addSkin(Assets::Texture* texture);
            void addFrame(const Mesh::TriangleSeries& triangleFans, const Mesh::TriangleSeries& triangleStrips);
        private:
            Renderer::MeshRenderer* doBuildRenderer(Renderer::Vbo& vbo, const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Md2Model__) */
