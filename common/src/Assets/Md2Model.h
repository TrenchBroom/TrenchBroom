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

#ifndef __TrenchBroom__Md2Model__
#define __TrenchBroom__Md2Model__

#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "StringUtils.h"
#include "VecMath.h"
#include "Renderer/TriangleMesh.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
    }
    
    namespace Assets {
        class TextureCollection;
        
        class Md2Model : public EntityModel {
        public:
            typedef Renderer::VertexSpecs::P3NT2 VertexSpec;
            typedef VertexSpec::Vertex Vertex;
            typedef Renderer::TriangleMesh<VertexSpec, const Assets::Texture*> Mesh;

            class Frame {
            private:
                Mesh::IndexedList m_triangleFans;
                Mesh::IndexedList m_triangleStrips;
                BBox3f m_bounds;
            public:
                Frame(Mesh::IndexedList& triangleFans, Mesh::IndexedList& triangleStrips);
                BBox3f transformedBounds(const Mat4x4f& transformation) const;
                
                const Mesh::IndexedList& triangleFans() const;
                const Mesh::IndexedList& triangleStrips() const;
                const BBox3f& bounds() const;
            private:
                void mergeBoundsWith(BBox3f& bounds, const Vertex::List& vertices) const;
                void mergeBoundsWith(BBox3f& bounds, const Vertex::List& vertices, const Mat4x4f& transformation) const;
            };

            typedef std::vector<Frame*> FrameList;
        private:
            String m_name;
            TextureCollection* m_skins;
            FrameList m_frames;
        public:
            Md2Model(const String& name, const TextureList& skins, const FrameList& frames);
            ~Md2Model();
        private:
            Renderer::TexturedTriangleMeshRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
            void doPrepare();
        };
    }
}

#endif /* defined(__TrenchBroom__Md2Model__) */
