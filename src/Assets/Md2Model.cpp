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

#include "Md2Model.h"
#include "CollectionUtils.h"
#include "Assets/AutoTexture.h"
#include "Renderer/MeshRenderer.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        Md2Model::Frame::Frame(const Mesh::TriangleSeries& i_triangleFans, const Mesh::TriangleSeries& i_triangleStrips) :
        triangleFans(i_triangleFans),
        triangleStrips(i_triangleStrips) {
            if (!triangleFans.empty())
                bounds.min = bounds.max = triangleFans.front().front().v1;
            else if (!triangleStrips.empty())
                bounds.min = bounds.max = triangleStrips.front().front().v1;
            mergeBoundsWith(bounds, triangleFans);
            mergeBoundsWith(bounds, triangleStrips);
        }

        BBox3f Md2Model::Frame::transformedBounds(const Mat4x4f& transformation) const {
            BBox3f transformedBounds;
            if (!triangleFans.empty())
                transformedBounds.min = transformedBounds.max = transformation * triangleFans.front().front().v1;
            else if (!triangleStrips.empty())
                transformedBounds.min = transformedBounds.max = transformation * triangleStrips.front().front().v1;
            mergeBoundsWith(transformedBounds, triangleFans, transformation);
            mergeBoundsWith(transformedBounds, triangleStrips, transformation);
            return transformedBounds;
        }

        void Md2Model::Frame::mergeBoundsWith(BBox3f& i_bounds, const Mesh::TriangleSeries& series) const {
            for (size_t i = 0; i < series.size(); ++i) {
                const VertexSpec::Vertex::List& vertices = series[i];
                for (size_t j = 0; j < vertices.size(); ++j) {
                    const VertexSpec::Vertex& vertex = vertices[j];
                    i_bounds.mergeWith(vertex.v1);
                }
            }
        }

        void Md2Model::Frame::mergeBoundsWith(BBox3f& i_bounds, const Mesh::TriangleSeries& series, const Mat4x4f& transformation) const {
            for (size_t i = 0; i < series.size(); ++i) {
                const VertexSpec::Vertex::List& vertices = series[i];
                for (size_t j = 0; j < vertices.size(); ++j) {
                    const VertexSpec::Vertex& vertex = vertices[j];
                    i_bounds.mergeWith(transformation * vertex.v1);
                }
            }
        }

        Md2Model::Md2Model(const String& name) :
        m_name(name) {}
        
        Md2Model::~Md2Model() {
            VectorUtils::clearAndDelete(m_skins);
        }

        void Md2Model::addSkin(Assets::Texture* texture) {
            m_skins.push_back(texture);
        }
        
        void Md2Model::addFrame(const Mesh::TriangleSeries& triangleFans, const Mesh::TriangleSeries& triangleStrips) {
            m_frames.push_back(Frame(triangleFans, triangleStrips));
        }
        
        Renderer::MeshRenderer* Md2Model::doBuildRenderer(Renderer::Vbo& vbo, const size_t skinIndex, const size_t frameIndex) const {
            assert(skinIndex < m_skins.size());
            assert(frameIndex < m_frames.size());

            const Assets::Texture* skin = m_skins[skinIndex];
            const Frame& frame = m_frames[frameIndex];
            
            Mesh mesh;
            mesh.addTriangleFans(skin, frame.triangleFans);
            mesh.addTriangleStrips(skin, frame.triangleStrips);
            
            return new Renderer::MeshRenderer(vbo, mesh);
        }
        
        BBox3f Md2Model::doGetBounds(const size_t skinIndex, const size_t frameIndex) const {
            assert(skinIndex < m_skins.size());
            assert(frameIndex < m_frames.size());
            
            const Frame& frame = m_frames[frameIndex];
            return frame.bounds;
        }
        
        BBox3f Md2Model::doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const {
            assert(skinIndex < m_skins.size());
            assert(frameIndex < m_frames.size());
            
            const Frame& frame = m_frames[frameIndex];
            return frame.transformedBounds(transformation);
        }
    }
}
