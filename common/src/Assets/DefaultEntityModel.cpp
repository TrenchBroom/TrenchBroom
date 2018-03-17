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

#include "DefaultEntityModel.h"

#include "Assets/TextureCollection.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        DefaultEntityModel::Frame::Frame(const String& name, const BBox3f& bounds, const DefaultEntityModel::VertexList& vertices, const DefaultEntityModel::Indices& indices) :
        m_name(name),
        m_bounds(bounds),
        m_vertices(std::move(vertices)),
        m_indices(std::move(indices)) {}

        const BBox3f& DefaultEntityModel::Frame::bounds() const {
            return m_bounds;
        }

        Renderer::TexturedIndexRangeRenderer* DefaultEntityModel::Frame::buildRenderer(Assets::Texture* skin) {
            const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
            return new Renderer::TexturedIndexRangeRenderer(vertexArray, texturedIndices);
        }

        DefaultEntityModel::DefaultEntityModel(const String& name) :
        m_name(name),
        m_skins(std::make_unique<TextureCollection>()) {}

        size_t DefaultEntityModel::frameCount() const {
            return m_frames.size();
        }

        size_t DefaultEntityModel::skinCount() const {
            return m_skins->textureCount();
        }

        void DefaultEntityModel::addSkin(Assets::Texture* skin) {
            m_skins->addTexture(skin);
        }

        void DefaultEntityModel::addFrame(const String& name, const DefaultEntityModel::VertexList& vertices, const DefaultEntityModel::Indices& indices) {
            const BBox3f bounds(std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent1());
            m_frames.push_back(std::make_unique<Frame>(name, bounds, vertices, indices));
        }

        Renderer::TexturedIndexRangeRenderer* DefaultEntityModel::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            const auto& textures = m_skins->textures();
            auto* skin = textures[skinIndex];
            return m_frames[frameIndex]->buildRenderer(skin);
        }

        BBox3f DefaultEntityModel::doGetBounds(const size_t /* skinIndex */, const size_t frameIndex) const {
            return m_frames[frameIndex]->bounds();
        }

        void DefaultEntityModel::doPrepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void DefaultEntityModel::doSetTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }
    }
}
