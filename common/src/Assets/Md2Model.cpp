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

#include "Md2Model.h"
#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Renderer/VertexArray.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace Assets {
        Md2Model::Frame::Frame(const VertexList& vertices, const Renderer::IndexRangeMap& indices) :
        m_vertices(vertices),
        m_indices(indices),
        m_bounds(m_vertices.begin(), m_vertices.end(), Renderer::GetVertexComponent1()) {}

        BBox3f Md2Model::Frame::transformedBounds(const Mat4x4f& transformation) const {
            BBox3f transformedBounds;
            
            VertexList::const_iterator it = m_vertices.begin();
            VertexList::const_iterator end = m_vertices.end();
            
            transformedBounds.min = transformedBounds.max = transformation * it->v1;
            while (++it != end)
                transformedBounds.mergeWith(transformation * it->v1);
            return transformedBounds;
        }

        const Md2Model::VertexList& Md2Model::Frame::vertices() const {
            return m_vertices;
        }
        
        const Renderer::IndexRangeMap& Md2Model::Frame::indices() const {
            return m_indices;
        }
        
        const BBox3f& Md2Model::Frame::bounds() const {
            return m_bounds;
        }

        Md2Model::Md2Model(const String& name, const TextureList& skins, const FrameList& frames) :
        m_name(name),
        m_skins(new TextureCollection(name, skins)),
        m_frames(frames) {}
        
        Md2Model::~Md2Model() {
            VectorUtils::clearAndDelete(m_frames);
            delete m_skins;
            m_skins = NULL;
        }

        Renderer::TexturedIndexRangeRenderer* Md2Model::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            const TextureList& textures = m_skins->textures();
            
            assert(skinIndex < textures.size());
            assert(frameIndex < m_frames.size());

            const Assets::Texture* skin = textures[skinIndex];
            const Frame* frame = m_frames[frameIndex];
            
            const VertexList& vertices = frame->vertices();
            const Renderer::IndexRangeMap& indices = frame->indices();
            
            const Renderer::VertexArray vertexArray = Renderer::VertexArray::ref(vertices);
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, indices);
            
            return new Renderer::TexturedIndexRangeRenderer(vertexArray, texturedIndices);
        }
        
        BBox3f Md2Model::doGetBounds(const size_t skinIndex, const size_t frameIndex) const {
            assert(skinIndex < m_skins->textures().size());
            assert(frameIndex < m_frames.size());
            
            const Frame* frame = m_frames[frameIndex];
            return frame->bounds();
        }
        
        BBox3f Md2Model::doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const {
            assert(skinIndex < m_skins->textures().size());
            assert(frameIndex < m_frames.size());
            
            const Frame* frame = m_frames[frameIndex];
            return frame->transformedBounds(transformation);
        }

        void Md2Model::doPrepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void Md2Model::doSetTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }
    }
}
