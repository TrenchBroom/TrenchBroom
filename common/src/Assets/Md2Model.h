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

#ifndef TrenchBroom_Md2Model
#define TrenchBroom_Md2Model

#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "StringUtils.h"
#include "VecMath.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/IndexRangeMap.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureCollection;
        
        class Md2Model : public EntityModel {
        public:
            typedef Renderer::VertexSpecs::P3NT2 VertexSpec;
            typedef VertexSpec::Vertex Vertex;
            typedef Vertex::List VertexList;
            
            class Frame {
            private:
                VertexList m_vertices;
                Renderer::IndexRangeMap m_indices;
                BBox3f m_bounds;
            public:
                Frame(const VertexList& vertices, const Renderer::IndexRangeMap& indices);
                BBox3f transformedBounds(const Mat4x4f& transformation) const;
                
                const VertexList& vertices() const;
                const Renderer::IndexRangeMap& indices() const;
                const BBox3f& bounds() const;
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
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
            void doPrepare(int minFilter, int magFilter);
            void doSetTextureMode(int minFilter, int magFilter);
        };
    }
}

#endif /* defined(TrenchBroom_Md2Model) */
