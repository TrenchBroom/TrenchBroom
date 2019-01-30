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
        EntityModel::Frame::Frame(const String& name, const vm::bbox3f& bounds) :
        m_name(name),
        m_bounds(bounds) {}

        const String& EntityModel::Frame::name() const {
            return m_name;
        }

        const vm::bbox3f& EntityModel::Frame::bounds() const {
            return m_bounds;
        }

        EntityModel::FrameData::FrameData(const EntityModel::VertexList& vertices) :
        m_vertices(std::move(vertices)) {}

        EntityModel::FrameData::~FrameData() {}

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::FrameData::buildRenderer(Assets::Texture* skin) {
            const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
            return doBuildRenderer(skin, vertexArray);
        }

        EntityModel::IndexedFrameData::IndexedFrameData(const EntityModel::VertexList& vertices, const EntityModel::Indices& indices) :
        FrameData(vertices),
        m_indices(indices) {}

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::IndexedFrameData::doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) {
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, texturedIndices);
        }

        EntityModel::TexturedFrameData::TexturedFrameData(const EntityModel::VertexList& vertices, const EntityModel::TexturedIndices& indices) :
        FrameData(vertices),
        m_indices(indices) {}

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::TexturedFrameData::doBuildRenderer(Assets::Texture* /* skin */, const Renderer::VertexArray& vertices) {
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, m_indices);
        }

        EntityModel::Surface::Surface(const String& name) :
        m_name(name),
        m_skins(std::make_unique<Assets::TextureCollection>()) {}

        void EntityModel::Surface::prepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void EntityModel::Surface::setTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }

        void EntityModel::Surface::addIndexedFrame(const VertexList& vertices, const Indices& indices) {
            m_frames.push_back(std::make_unique<IndexedFrameData>(vertices, indices));
        }

        void EntityModel::Surface::addTexturedFrame(const VertexList& vertices, const TexturedIndices& indices) {
            m_frames.push_back(std::make_unique<TexturedFrameData>(vertices, indices));
        }

        void EntityModel::Surface::addSkin(Assets::Texture* skin) {
            m_skins->addTexture(skin);
        }

        size_t EntityModel::Surface::frameCount() const {
            return m_frames.size();
        }

        size_t EntityModel::Surface::skinCount() const {
            return m_skins->textureCount();
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::Surface::buildRenderer(size_t skinIndex, size_t frameIndex) {
            if (skinIndex >= skinCount() || frameIndex >= frameCount()) {
                return nullptr;
            } else {
                const auto& textures = m_skins->textures();
                auto* skin = textures[skinIndex];
                return m_frames[frameIndex]->buildRenderer(skin);
            }
        }

        EntityModel::EntityModel(const String& name) :
        m_name(name),
        m_prepared(false) {}

        Renderer::TexturedRenderer* EntityModel::buildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            std::vector<std::unique_ptr<Renderer::TexturedIndexRangeRenderer>> renderers;
            for (const auto& surface : m_surfaces) {
                auto renderer = surface->buildRenderer(skinIndex, frameIndex);
                if (renderer != nullptr) {
                    renderers.push_back(std::move(renderer));
                }
            }
            if (renderers.empty()) {
                return nullptr;
            } else {
                return new Renderer::MultiTexturedIndexRangeRenderer(std::move(renderers));
            }
        }

        vm::bbox3f EntityModel::bounds(const size_t /* skinIndex */, const size_t frameIndex) const {
            if (frameIndex >= m_frames.size()) {
                return vm::bbox3f(8.0f);
            } else {
                return m_frames[frameIndex]->bounds();
            }
        }

        bool EntityModel::prepared() const {
            return m_prepared;
        }

        void EntityModel::prepare(const int minFilter, const int magFilter) {
            if (!m_prepared) {
                for (auto& surface : m_surfaces) {
                    surface->prepare(minFilter, magFilter);
                }
                m_prepared = true;
            }
        }

        void EntityModel::setTextureMode(const int minFilter, const int magFilter) {
            for (auto& surface : m_surfaces) {
                surface->setTextureMode(minFilter, magFilter);
            }
        }

        EntityModel::Frame& EntityModel::addFrame(const String& name, const vm::bbox3f& bounds) {
            m_frames.push_back(std::make_unique<Frame>(name, bounds));
            return *m_frames.back();
        }

        EntityModel::Surface& EntityModel::addSurface(const String& name) {
            m_surfaces.push_back(std::make_unique<Surface>(name));
            return *m_surfaces.back();
        }

        size_t EntityModel::frameCount() const {
            return m_frames.size();
        }

        size_t EntityModel::surfaceCount() const {
            return m_surfaces.size();
        }

        std::vector<const EntityModel::Frame*> EntityModel::frames() const {
            std::vector<const EntityModel::Frame*> result;
            result.reserve(frameCount());
            for (const auto& frame : m_frames) {
                result.push_back(frame.get());
            }
            return result;
        }

        std::vector<const EntityModel::Surface*> EntityModel::surfaces() const {
            std::vector<const EntityModel::Surface*> result;
            result.reserve(surfaceCount());
            for (const auto& surface : m_surfaces) {
                result.push_back(surface.get());
            }
            return result;
        }
    }
}
