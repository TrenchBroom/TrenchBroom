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

#include "EntityModel.h"

#include "Renderer/TexturedIndexRangeRenderer.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

namespace TrenchBroom {
    namespace Assets {
        EntityModel::Frame::Frame(const String& name, const vm::bbox3f& bounds, const EntityModel::VertexList& vertices) :
        m_name(name),
        m_bounds(bounds),
        m_vertices(std::move(vertices)) {}

        EntityModel::Frame::~Frame() {}

        const vm::bbox3f& EntityModel::Frame::bounds() const {
            return m_bounds;
        }

        Renderer::TexturedIndexRangeRenderer* EntityModel::Frame::buildRenderer(Assets::Texture* skin) {
            const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
            return doBuildRenderer(skin, vertexArray);
        }

        EntityModel::IndexedFrame::IndexedFrame(const String& name, const vm::bbox3f& bounds, const EntityModel::VertexList& vertices, const EntityModel::Indices& indices) :
                Frame(name, bounds, vertices),
                m_indices(indices) {}

        Renderer::TexturedIndexRangeRenderer* EntityModel::IndexedFrame::doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) {
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
            return new Renderer::TexturedIndexRangeRenderer(vertices, texturedIndices);
        }

        EntityModel::TexturedFrame::TexturedFrame(const String& name, const vm::bbox3f& bounds, const EntityModel::VertexList& vertices, const EntityModel::TexturedIndices& indices) :
                Frame(name, bounds, vertices),
                m_indices(indices) {}

        Renderer::TexturedIndexRangeRenderer* EntityModel::TexturedFrame::doBuildRenderer(Assets::Texture* /* skin */, const Renderer::VertexArray& vertices) {
            return new Renderer::TexturedIndexRangeRenderer(vertices, m_indices);
        }
        
        EntityModel::EntityModel(const String& name) :
        m_name(name),
        m_skins(std::make_unique<Assets::TextureCollection>()),
        m_prepared(false) {}

        Renderer::TexturedIndexRangeRenderer * EntityModel::buildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            ensure(skinIndex < skinCount(), "skin index out of range");
            ensure(frameIndex < frameCount(), "frame index out of range");

            const auto& textures = m_skins->textures();
            auto* skin = textures[skinIndex];
            return m_frames[frameIndex]->buildRenderer(skin);
        }

        vm::bbox3f EntityModel::bounds(const size_t skinIndex, const size_t frameIndex) const {
            ensure(skinIndex < skinCount(), "skin index out of range");
            ensure(frameIndex < frameCount(), "frame index out of range");

            return m_frames[frameIndex]->bounds();
        }

        size_t EntityModel::frameCount() const {
            return m_frames.size();
        }

        size_t EntityModel::skinCount() const {
            return m_skins->textureCount();
        }

        bool EntityModel::prepared() const {
            return m_prepared;
        }

        Assets::Texture* EntityModel::skin(const size_t index) const {
            return m_skins->textureByIndex(index);
        }

        void EntityModel::prepare(const int minFilter, const int magFilter) {
            if (!m_prepared) {
                m_skins->prepare(minFilter, magFilter);
                m_prepared = true;
            }
        }

        void EntityModel::setTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }

        void EntityModel::addSkin(Assets::Texture* skin) {
            m_skins->addTexture(skin);
        }

        void EntityModel::addFrame(const String& name, const EntityModel::VertexList& vertices, const EntityModel::Indices& indices) {
            const auto bounds = vm::bbox3f::mergeAll(std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent1());
            m_frames.push_back(std::make_unique<IndexedFrame>(name, bounds, vertices, indices));
        }

        void EntityModel::addFrame(const String& name, const EntityModel::VertexList& vertices, const EntityModel::TexturedIndices& indices) {
            const auto bounds = vm::bbox3f::mergeAll(std::begin(vertices), std::end(vertices), Renderer::GetVertexComponent1());
            m_frames.push_back(std::make_unique<TexturedFrame>(name, bounds, vertices, indices));
        }
    }
}
