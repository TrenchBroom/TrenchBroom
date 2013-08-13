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

#include "Md2Model.h"
#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Assets {
        Md2Model::Frame::Frame(const Mesh::TriangleSeries& i_triangleFans, const Mesh::TriangleSeries& i_triangleStrips) :
        triangleFans(i_triangleFans),
        triangleStrips(i_triangleStrips) {}

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
        }
        
        BBox3f Md2Model::doGetBounds(const size_t skinIndex, const size_t frameIndex) const {
        }
        
        BBox3f Md2Model::doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const {
        }
    }
}
