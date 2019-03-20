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
#include <vecmath/intersection.h>

#include <limits>

namespace TrenchBroom {
    namespace Assets {
        EntityModelFrame::EntityModelFrame(const size_t index) :
        m_index(index) {}

        EntityModelFrame::~EntityModelFrame() = default;

        size_t EntityModelFrame::index() const {
            return m_index;
        }

        EntityModel::LoadedFrame::LoadedFrame(const size_t index, const String& name, const vm::bbox3f& bounds) :
        EntityModelFrame(index),
        m_name(name),
        m_bounds(bounds) {}

        bool EntityModel::LoadedFrame::loaded() const {
            return true;
        }

        const String& EntityModel::LoadedFrame::name() const {
            return m_name;
        }

        const vm::bbox3f& EntityModel::LoadedFrame::bounds() const {
            return m_bounds;
        }

        float EntityModel::LoadedFrame::intersect(const vm::ray3f& ray) const {
            auto closestDistance = vm::nan<float>();

            const auto candidates = m_spacialTree.findIntersectors(ray);
            for (const auto& triangle : candidates) {
                const auto& p1 = triangle[0];
                const auto& p2 = triangle[1];
                const auto& p3 = triangle[2];
                closestDistance = vm::safeMin(closestDistance, vm::intersect(ray, p1, p2, p3));
            }

            return closestDistance;
        }

        void EntityModel::LoadedFrame::addToSpacialTree(const EntityModel::VertexList& vertices, const PrimType primType, const size_t index, const size_t count) {
            switch (primType) {
                case GL_POINTS:
                case GL_LINES:
                case GL_LINE_STRIP:
                case GL_LINE_LOOP:
                    break;
                case GL_TRIANGLES: {
                    assert(count % 3 == 0);
                    for (size_t i = 0; i < count; i += 3) {
                        vm::bbox3f::builder bounds;
                        const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p3);
                        m_spacialTree.insert(bounds.bounds(), {{ p1, p2, p3 }});
                    }
                    break;
                }
                case GL_POLYGON:
                case GL_TRIANGLE_FAN: {
                    assert(count > 2);
                    for (size_t i = 1; i < count - 1; ++i) {
                        vm::bbox3f::builder bounds;
                        const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + 0]);
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p2);
                        m_spacialTree.insert(bounds.bounds(), { { p1, p2, p3 } });
                    }
                    break;
                }
                case GL_QUADS:
                case GL_QUAD_STRIP:
                case GL_TRIANGLE_STRIP: {
                    assert(count > 2);
                    for (size_t i = 0; i < count-2; ++i) {
                        vm::bbox3f::builder bounds;
                        const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p2);
                        if (i % 2 == 0) {
                            m_spacialTree.insert(bounds.bounds(), {{ p1, p2, p3 }});
                        } else {
                            m_spacialTree.insert(bounds.bounds(), {{ p1, p3, p2 }});
                        }
                    }
                    break;
                }
                switchDefault();
            }
        }

        EntityModel::UnloadedFrame::UnloadedFrame(size_t index) :
        EntityModelFrame(index) {}

        bool EntityModel::UnloadedFrame::loaded() const {
            return false;
        }

        const String& EntityModel::UnloadedFrame::name() const {
            static const String name = "Unloaded frame";
            return name;
        }

        const vm::bbox3f& EntityModel::UnloadedFrame::bounds() const {
            static const auto bounds = vm::bbox3f(8.0f);
            return bounds;
        }

        float EntityModel::UnloadedFrame::intersect(const vm::ray3f& ray) const {
            return vm::nan<float>();
        }

        EntityModel::Mesh::Mesh(const EntityModel::VertexList& vertices) :
        m_vertices(vertices) {}

        EntityModel::Mesh::~Mesh() = default;

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::Mesh::buildRenderer(Assets::Texture* skin) {
            const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
            return doBuildRenderer(skin, vertexArray);
        }

        EntityModel::IndexedMesh::IndexedMesh(LoadedFrame& frame, const EntityModel::VertexList& vertices, const EntityModel::Indices& indices) :
        Mesh(vertices),
        m_indices(indices) {
            m_indices.forEachPrimitive([&frame, &vertices](const PrimType primType, const size_t index, const size_t count) {
                frame.addToSpacialTree(vertices, primType, index, count);
            });
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::IndexedMesh::doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) {
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, texturedIndices);
        }

        EntityModel::TexturedMesh::TexturedMesh(LoadedFrame& frame, const EntityModel::VertexList& vertices, const EntityModel::TexturedIndices& indices) :
        Mesh(vertices),
        m_indices(indices) {
            m_indices.forEachPrimitive([&frame, &vertices](const Assets::Texture* texture, const PrimType primType, const size_t index, const size_t count) {
                frame.addToSpacialTree(vertices, primType, index, count);
            });
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::TexturedMesh::doBuildRenderer(Assets::Texture* /* skin */, const Renderer::VertexArray& vertices) {
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, m_indices);
        }

        EntityModel::Surface::Surface(const String& name, const size_t frameCount) :
        m_name(name),
        m_meshes(frameCount),
        m_skins(std::make_unique<Assets::TextureCollection>()) {}

        const String& EntityModel::Surface::name() const {
            return m_name;
        }

        void EntityModel::Surface::prepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void EntityModel::Surface::setTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }

        void EntityModel::Surface::addIndexedMesh(LoadedFrame& frame, const VertexList& vertices, const Indices& indices) {
            assert(frame.index() < frameCount());
            m_meshes[frame.index()] = std::make_unique<IndexedMesh>(frame, vertices, indices);
        }

        void EntityModel::Surface::addTexturedMesh(LoadedFrame& frame, const VertexList& vertices, const TexturedIndices& indices) {
            assert(frame.index() < frameCount());
            m_meshes[frame.index()] = std::make_unique<TexturedMesh>(frame, vertices, indices);
        }

        void EntityModel::Surface::addSkin(Assets::Texture* skin) {
            m_skins->addTexture(skin);
        }

        size_t EntityModel::Surface::frameCount() const {
            return m_meshes.size();
        }

        size_t EntityModel::Surface::skinCount() const {
            return m_skins->textureCount();
        }

        Assets::Texture* EntityModel::Surface::skin(const String& name) const {
            return m_skins->textureByName(name);
        }

        Assets::Texture* EntityModel::Surface::skin(const size_t index) const {
            if (index >= skinCount()) {
                return nullptr;
            } else {
                const auto& textures = m_skins->textures();
                return textures[index];
            }
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::Surface::buildRenderer(size_t skinIndex, size_t frameIndex) {
            if (skinIndex >= skinCount() || frameIndex >= frameCount() || m_meshes[frameIndex] == nullptr) {
                return nullptr;
            } else {
                const auto& textures = m_skins->textures();
                auto* skin = textures[skinIndex];
                return m_meshes[frameIndex]->buildRenderer(skin);
            }
        }

        EntityModel::EntityModel(const String& name) :
        m_name(name),
        m_prepared(false) {}

        std::unique_ptr<Renderer::TexturedRenderer> EntityModel::buildRenderer(const size_t skinIndex, const size_t frameIndex) const {
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
                return std::make_unique<Renderer::MultiTexturedIndexRangeRenderer>(std::move(renderers));
            }
        }

        vm::bbox3f EntityModel::bounds(const size_t frameIndex) const {
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

        void EntityModel::addFrames(const size_t count) {
            for (size_t i = 0; i < count; ++i) {
                m_frames.emplace_back(std::make_unique<UnloadedFrame>(frameCount()));
            }
        }

        EntityModel::LoadedFrame& EntityModel::loadFrame(const size_t frameIndex, const String& name, const vm::bbox3f& bounds) {
            if (frameIndex >= frameCount()) {
                AssetException ex;
                ex << "Frame index " << frameIndex << " is out of bounds (frame count = " << frameCount() << ")";
                throw ex;
            }

            auto frame = std::make_unique<LoadedFrame>(frameIndex, name, bounds);
            auto& result = *frame;
            m_frames[frameIndex] = std::move(frame);
            return result;
        }

        EntityModel::Surface& EntityModel::addSurface(const String& name) {
            m_surfaces.push_back(std::make_unique<Surface>(name, frameCount()));
            return *m_surfaces.back();
        }

        size_t EntityModel::frameCount() const {
            return m_frames.size();
        }

        size_t EntityModel::surfaceCount() const {
            return m_surfaces.size();
        }

        std::vector<const EntityModelFrame*> EntityModel::frames() const {
            std::vector<const EntityModelFrame*> result;
            result.reserve(frameCount());
            for (const auto& frame : m_frames) {
                result.push_back(frame.get());
            }
            return result;
        }

        std::vector<EntityModelFrame*> EntityModel::frames() {
            std::vector<EntityModelFrame*> result;
            result.reserve(frameCount());
            for (const auto& frame : m_frames) {
                result.push_back(frame.get());
            }
            return result;
        }

        std::vector<const EntityModel::Surface*> EntityModel::surfaces() const {
            std::vector<const Surface*> result;
            result.reserve(surfaceCount());
            for (const auto& surface : m_surfaces) {
                result.push_back(surface.get());
            }
            return result;
        }

        const EntityModelFrame* EntityModel::frame(const String& name) const {
            for (const auto& frame : m_frames) {
                if (frame->name() == name) {
                    return frame.get();
                }
            }
            return nullptr;
        }

        const EntityModelFrame* EntityModel::frame(const size_t index) const {
            if (index >= frameCount()) {
                return nullptr;
            } else {
                return m_frames[index].get();
            }
        }

        EntityModel::Surface& EntityModel::surface(const size_t index) {
            if (index >= surfaceCount()) {
                throw std::out_of_range("Surface index is out of bounds");
            }
            return *m_surfaces[index];
        }

        const EntityModel::Surface* EntityModel::surface(const String& name) const {
            for (const auto& surface : m_surfaces) {
                if (surface->name() == name) {
                    return surface.get();
                }
            }
            return nullptr;
        }
    }
}
