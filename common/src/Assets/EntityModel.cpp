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
        EntityModel::Frame::Frame(const String& name, const vm::bbox3f& bounds) :
        m_name(name),
        m_bounds(bounds) {}

        const String& EntityModel::Frame::name() const {
            return m_name;
        }

        const vm::bbox3f& EntityModel::Frame::bounds() const {
            return m_bounds;
        }

        EntityModel::Mesh::Mesh(const EntityModel::VertexList& vertices) :
        m_vertices(std::move(vertices)) {}

        EntityModel::Mesh::~Mesh() = default;

        float EntityModel::Mesh::intersect(const vm::ray3f& ray) const {
            auto closestDistance = vm::nan<float>();

            const auto candidates = m_spacialTree.findIntersectors(ray);
            for (const auto& candidate : candidates) {
                const auto& p1 = m_vertices[candidate[0]];
                const auto& p2 = m_vertices[candidate[1]];
                const auto& p3 = m_vertices[candidate[2]];

                closestDistance = vm::safeMin(closestDistance, vm::intersect(ray, p1.v1, p2.v1, p3.v1));
            }

            return closestDistance;
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::Mesh::buildRenderer(Assets::Texture* skin) {
            const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
            return doBuildRenderer(skin, vertexArray);
        }

        void EntityModel::Mesh::addToSpacialTree(const PrimType primType, const size_t index, const size_t count) {
            switch (primType) {
                case GL_POINTS:
                case GL_LINES:
                case GL_LINE_STRIP:
                case GL_LINE_LOOP:
                    break;
                case GL_TRIANGLES: {
                    assert(count == 3);
                    vm::bbox3f::builder bounds;
                    for (size_t i = 0; i < count; ++i) {
                        bounds.add(m_vertices[index + i].v1);
                    }
                    m_spacialTree.insert(bounds.bounds(), { index + 0, index + 1, index + 2 });
                }
                case GL_POLYGON:
                case GL_TRIANGLE_FAN:
                    assert(count > 2);
                    for (size_t i = 1; i < count-1; ++i) {
                        vm::bbox3f::builder bounds;
                        bounds.add(m_vertices[index + 0].v1);
                        bounds.add(m_vertices[index + i].v1);
                        bounds.add(m_vertices[index + i + 1].v1);
                        m_spacialTree.insert(bounds.bounds(), { index + 0, index + i, index + i + 1 });
                    }
                case GL_QUADS:
                case GL_QUAD_STRIP:
                case GL_TRIANGLE_STRIP: {
                    assert(count > 2);
                    for (size_t i = 0; i < count-2; ++i) {
                        vm::bbox3f::builder bounds;
                        bounds.add(m_vertices[index + i + 0].v1);
                        bounds.add(m_vertices[index + i + 1].v1);
                        bounds.add(m_vertices[index + i + 2].v1);
                        if (i % 2 == 0) {
                            m_spacialTree.insert(bounds.bounds(), { index + i + 0, index + i + 1, index + i + 2 });
                        } else {
                            m_spacialTree.insert(bounds.bounds(), { index + i + 0, index + i + 2, index + i + 1 });
                        }
                    }
                }
                    switchDefault();
            }
        }

        EntityModel::IndexedMesh::IndexedMesh(const EntityModel::VertexList& vertices, const EntityModel::Indices& indices) :
        Mesh(vertices),
        m_indices(indices) {
            // Populate the AABB tree used for picking the primitives
            m_indices.forEachPrimitive([this](const PrimType primType, const size_t index, const size_t count) {
                addToSpacialTree(primType, index, count);
            });
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::IndexedMesh::doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) {
            const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, texturedIndices);
        }

        EntityModel::TexturedMesh::TexturedMesh(const EntityModel::VertexList& vertices, const EntityModel::TexturedIndices& indices) :
        Mesh(vertices),
        m_indices(indices) {
            m_indices.forEachPrimitive([this](const Assets::Texture* texture, const PrimType primType, const size_t index, const size_t count) {
                addToSpacialTree(primType, index, count);
            });
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::TexturedMesh::doBuildRenderer(Assets::Texture* /* skin */, const Renderer::VertexArray& vertices) {
            return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, m_indices);
        }

        EntityModel::Surface::Surface(const String& name) :
        m_name(name),
        m_skins(std::make_unique<Assets::TextureCollection>()) {}

        const String& EntityModel::Surface::name() const {
            return m_name;
        }

        float EntityModel::Surface::intersect(const vm::ray3f& ray, const size_t frameIndex) const {
            assert(frameIndex < m_meshes.size());
            return m_meshes[frameIndex]->intersect(ray);
        }

        void EntityModel::Surface::prepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void EntityModel::Surface::setTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }

        void EntityModel::Surface::addIndexedMesh(const VertexList& vertices, const Indices& indices) {
            m_meshes.emplace_back(std::make_unique<IndexedMesh>(vertices, indices));
        }

        void EntityModel::Surface::addTexturedMesh(const VertexList& vertices, const TexturedIndices& indices) {
            m_meshes.emplace_back(std::make_unique<TexturedMesh>(vertices, indices));
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

        const Assets::Texture* EntityModel::Surface::skin(const String& name) const {
            return m_skins->textureByName(name);
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModel::Surface::buildRenderer(size_t skinIndex, size_t frameIndex) {
            if (skinIndex >= skinCount() || frameIndex >= frameCount()) {
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

        vm::bbox3f EntityModel::bounds(const size_t frameIndex) const {
            if (frameIndex >= m_frames.size()) {
                return vm::bbox3f(8.0f);
            } else {
                return m_frames[frameIndex]->bounds();
            }
        }

        float EntityModel::intersect(const vm::ray3f& ray, const size_t frameIndex) const {
            if (frameIndex >= m_frames.size()) {
                return vm::nan<FloatType>();
            } else {
                auto closestDistance = vm::nan<float>();
                for (const auto& surface : m_surfaces) {
                    closestDistance = vm::safeMin(closestDistance, surface->intersect(ray, frameIndex));
                }
                return closestDistance;
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

        const EntityModel::Frame* EntityModel::frame(const String& name) const {
            for (const auto& frame : m_frames) {
                if (frame->name() == name) {
                    return frame.get();
                }
            }
            return nullptr;
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
