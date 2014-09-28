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
#include "Renderer/MeshRenderer.h"

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace Assets {
        Md2Model::Frame::Frame(Mesh::IndexedList& triangleFans, Mesh::IndexedList& triangleStrips) {
            using std::swap;

            swap(m_triangleFans, triangleFans);
            swap(m_triangleStrips, triangleStrips);
            
            if (!m_triangleFans.empty())
                m_bounds.min = m_bounds.max = m_triangleFans.vertices().front().v1;
            else if (!m_triangleStrips.empty())
                m_bounds.min = m_bounds.max = m_triangleStrips.vertices().front().v1;
            mergeBoundsWith(m_bounds, m_triangleFans.vertices());
            mergeBoundsWith(m_bounds, m_triangleStrips.vertices());
        }

        BBox3f Md2Model::Frame::transformedBounds(const Mat4x4f& transformation) const {
            BBox3f transformedBounds;
            if (!m_triangleFans.empty())
                transformedBounds.min = transformedBounds.max = transformation * m_triangleFans.vertices().front().v1;
            else if (!m_triangleStrips.empty())
                transformedBounds.min = transformedBounds.max = transformation * m_triangleStrips.vertices().front().v1;
            mergeBoundsWith(transformedBounds, m_triangleFans.vertices(), transformation);
            mergeBoundsWith(transformedBounds, m_triangleStrips.vertices(), transformation);
            return transformedBounds;
        }

        const Md2Model::Mesh::IndexedList& Md2Model::Frame::triangleFans() const {
            return m_triangleFans;
        }
        
        const Md2Model::Mesh::IndexedList& Md2Model::Frame::triangleStrips() const {
            return m_triangleStrips;
        }
        
        const BBox3f& Md2Model::Frame::bounds() const {
            return m_bounds;
        }

        void Md2Model::Frame::mergeBoundsWith(BBox3f& bounds, const Vertex::List& vertices) const {
            for (size_t i = 0; i < vertices.size(); ++i)
                bounds.mergeWith(vertices[i].v1);
        }

        void Md2Model::Frame::mergeBoundsWith(BBox3f& bounds, const Vertex::List& vertices, const Mat4x4f& transformation) const {
            for (size_t i = 0; i < vertices.size(); ++i)
                bounds.mergeWith(transformation * vertices[i].v1);
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

        Renderer::MeshRenderer* Md2Model::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            const TextureList& textures = m_skins->textures();
            
            assert(skinIndex < textures.size());
            assert(frameIndex < m_frames.size());

            const Assets::Texture* skin = textures[skinIndex];
            const Frame* frame = m_frames[frameIndex];
            const Mesh::IndexedList& fans = frame->triangleFans();
            const Mesh::IndexedList& strips = frame->triangleStrips();
            
            Mesh::MeshSize size;
            size.addFans(skin, fans.vertexCount(), fans.primCount());
            size.addStrips(skin, strips.vertexCount(), strips.primCount());
            
            Mesh mesh(size);
            mesh.addTriangleFans(skin, fans);
            mesh.addTriangleStrips(skin, fans);
            
            return new Renderer::MeshRenderer(mesh);
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

        
        void Md2Model::doPrepare() {
            m_skins->prepare(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        }
    }
}
