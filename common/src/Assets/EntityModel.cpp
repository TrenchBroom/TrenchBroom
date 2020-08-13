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

#include "AABBTree.h"
#include "Assets/TextureCollection.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/PrimType.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/intersection.h>

#include <string>

namespace TrenchBroom {
    namespace Assets {
        // EntityModelFrame

        EntityModelFrame::EntityModelFrame(const size_t index) :
        m_index(index) {}

        EntityModelFrame::~EntityModelFrame() = default;

        size_t EntityModelFrame::index() const {
            return m_index;
        }

        // EntityModel::LoadedFrame

        EntityModelLoadedFrame::EntityModelLoadedFrame(const size_t index, const std::string& name, const vm::bbox3f& bounds, const PitchType pitchType) :
        EntityModelFrame(index),
        m_name(name),
        m_bounds(bounds),
        m_pitchType(pitchType),
        m_spacialTree(std::make_unique<SpacialTree>()) {}

        EntityModelLoadedFrame::~EntityModelLoadedFrame() = default;

        bool EntityModelLoadedFrame::loaded() const {
            return true;
        }

        const std::string& EntityModelLoadedFrame::name() const {
            return m_name;
        }

        const vm::bbox3f& EntityModelLoadedFrame::bounds() const {
            return m_bounds;
        }

        PitchType EntityModelLoadedFrame::pitchType() const {
            return m_pitchType;
        }

        float EntityModelLoadedFrame::intersect(const vm::ray3f& ray) const {
            auto closestDistance = vm::nan<float>();

            const auto candidates = m_spacialTree->findIntersectors(ray);
            for (const TriNum triNum : candidates) {
                const vm::vec3f& p1 = m_tris[triNum * 3 + 0];
                const vm::vec3f& p2 = m_tris[triNum * 3 + 1];
                const vm::vec3f& p3 = m_tris[triNum * 3 + 2];
                closestDistance = vm::safe_min(closestDistance, vm::intersect_ray_triangle(ray, p1, p2, p3));
            }

            return closestDistance;
        }

        void EntityModelLoadedFrame::addToSpacialTree(const std::vector<EntityModelVertex>& vertices, const Renderer::PrimType primType, const size_t index, const size_t count) {
            switch (primType) {
                case Renderer::PrimType::Points:
                case Renderer::PrimType::Lines:
                case Renderer::PrimType::LineStrip:
                case Renderer::PrimType::LineLoop:
                    break;
                case Renderer::PrimType::Triangles: {
                    assert(count % 3 == 0);
                    m_tris.reserve(m_tris.size() + count);
                    for (size_t i = 0; i < count; i += 3) {
                        vm::bbox3f::builder bounds;
                        const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p3);

                        const size_t triIndex = m_tris.size() / 3u;
                        m_tris.push_back(p1);
                        m_tris.push_back(p2);
                        m_tris.push_back(p3);
                        m_spacialTree->insert(bounds.bounds(), triIndex);
                    }
                    break;
                }
                case Renderer::PrimType::Polygon:
                case Renderer::PrimType::TriangleFan: {
                    assert(count > 2);
                    m_tris.reserve(m_tris.size() + (count - 2) * 3);

                    const auto& p1 = Renderer::getVertexComponent<0>(vertices[index]);
                    for (size_t i = 1; i < count - 1; ++i) {
                        vm::bbox3f::builder bounds;
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p3);

                        const size_t triIndex = m_tris.size() / 3u;
                        m_tris.push_back(p1);
                        m_tris.push_back(p2);
                        m_tris.push_back(p3);
                        m_spacialTree->insert(bounds.bounds(), triIndex);
                    }
                    break;
                }
                case Renderer::PrimType::Quads:
                case Renderer::PrimType::QuadStrip:
                case Renderer::PrimType::TriangleStrip: {
                    assert(count > 2);
                    m_tris.reserve(m_tris.size() + (count - 2) * 3);
                    for (size_t i = 0; i < count-2; ++i) {
                        vm::bbox3f::builder bounds;
                        const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
                        const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
                        const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
                        bounds.add(p1);
                        bounds.add(p2);
                        bounds.add(p3);

                        const size_t triIndex = m_tris.size() / 3u;
                        if (i % 2 == 0) {
                            m_tris.push_back(p1);
                            m_tris.push_back(p2);
                            m_tris.push_back(p3);
                        } else {
                            m_tris.push_back(p1);
                            m_tris.push_back(p3);
                            m_tris.push_back(p2);
                        }
                        m_spacialTree->insert(bounds.bounds(), triIndex);
                    }
                    break;
                }
                switchDefault();
            }
        }

        // EntityModel::UnloadedFrame

        /**
         * A frame of the model in its unloaded state.
         */
        class EntityModelUnloadedFrame : public EntityModelFrame {
        public:
            /**
             * Creates a new frame with the given index.
             *
             * @param index the index of this frame
             */
            explicit EntityModelUnloadedFrame(const size_t index) :
            EntityModelFrame(index) {}

            bool loaded() const override {
                return false;
            }

            const std::string& name() const override {
                static const std::string name = "Unloaded frame";
                return name;
            }

            const vm::bbox3f& bounds() const override {
                static const auto bounds = vm::bbox3f(8.0f);
                return bounds;
            }

            PitchType pitchType() const override {
                return PitchType::Normal;
            }

            float intersect(const vm::ray3f& /* ray */) const override {
                return vm::nan<float>();
            }
        };

        // EntityModel::Mesh

        /**
         * The mesh associated with a frame and a surface.
         */
        class EntityModelMesh {
        private:
            std::vector<EntityModelVertex> m_vertices;
        protected:
            /**
             * Creates a new frame mesh that uses the given vertices.
             *
             * @param vertices the vertices
             */
            explicit EntityModelMesh(const std::vector<EntityModelVertex>& vertices) :
            m_vertices(vertices) {}
        public:
            virtual ~EntityModelMesh() = default;
        public:
            /**
             * Returns a renderer that renders this mesh with the given texture.
             *
             * @param skin the texture to use when rendering the mesh
             * @return the renderer
             */
            std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(Assets::Texture* skin) {
                const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
                return doBuildRenderer(skin, vertexArray);
            }
        private:
            /**
             * Creates and returns the actual mesh renderer
             *
             * @param skin the skin to use when rendering the mesh
             * @param vertices the vertices associated with this mesh
             * @return the renderer
             */
            virtual std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) = 0;
        };

        // EntityModel::IndexedMesh

        /**
         * A model frame mesh for indexed rendering. Stores vertices and vertex indices.
         */
        class EntityModelIndexedMesh : public EntityModelMesh {
        private:
            EntityModelIndices m_indices;
        public:
            /**
             * Creates a new frame mesh with the given vertices and indices.
             *
             * @param frame the frame to which this mesh belongs
             * @param vertices the vertices
             * @param indices the indices
             */
            EntityModelIndexedMesh(EntityModelLoadedFrame& frame, const std::vector<EntityModelVertex>& vertices, const EntityModelIndices& indices) :
            EntityModelMesh(vertices),
            m_indices(indices) {
                m_indices.forEachPrimitive([&frame, &vertices](const Renderer::PrimType primType, const size_t index, const size_t count) {
                    frame.addToSpacialTree(vertices, primType, index, count);
                });
        }
        private:
            std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override {
                const Renderer::TexturedIndexRangeMap texturedIndices(skin, m_indices);
                return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, texturedIndices);
            }
        };

        // EntityModel::TexturedMesh

        /**
         * A model frame mesh for per texture indexed rendering. Stores vertices and per texture indices.
         */
        class EntityModelTexturedMesh : public EntityModelMesh {
        private:
            EntityModelTexturedIndices m_indices;
        public:
            /**
             * Creates a new frame mesh with the given vertices and per texture indices.
             *
             * @param frame the frame to which this mesh belongs
             * @param vertices the vertices
             * @param indices the per texture indices
             */
            EntityModelTexturedMesh(EntityModelLoadedFrame& frame, const std::vector<EntityModelVertex>& vertices, const EntityModelTexturedIndices& indices) :
            EntityModelMesh(vertices),
            m_indices(indices) {
                m_indices.forEachPrimitive([&frame, &vertices](const Assets::Texture* /* texture */, const Renderer::PrimType primType, const size_t index, const size_t count) {
                    frame.addToSpacialTree(vertices, primType, index, count);
                });
            }
        private:
            std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* /* skin */, const Renderer::VertexArray& vertices) override {
                return std::make_unique<Renderer::TexturedIndexRangeRenderer>(vertices, m_indices);
            }
        };

        // EntityModel::Surface

        EntityModelSurface::EntityModelSurface(const std::string& name, const size_t frameCount) :
        m_name(name),
        m_meshes(frameCount),
        m_skins(std::make_unique<Assets::TextureCollection>()) {}

        EntityModelSurface::~EntityModelSurface() = default;

        const std::string& EntityModelSurface::name() const {
            return m_name;
        }

        void EntityModelSurface::prepare(const int minFilter, const int magFilter) {
            m_skins->prepare(minFilter, magFilter);
        }

        void EntityModelSurface::setTextureMode(const int minFilter, const int magFilter) {
            m_skins->setTextureMode(minFilter, magFilter);
        }

        void EntityModelSurface::addIndexedMesh(EntityModelLoadedFrame& frame, const std::vector<EntityModelVertex>& vertices, const EntityModelIndices& indices) {
            assert(frame.index() < frameCount());
            m_meshes[frame.index()] = std::make_unique<EntityModelIndexedMesh>(frame, vertices, indices);
        }

        void EntityModelSurface::addTexturedMesh(EntityModelLoadedFrame& frame, const std::vector<EntityModelVertex>& vertices, const EntityModelTexturedIndices& indices) {
            assert(frame.index() < frameCount());
            m_meshes[frame.index()] = std::make_unique<EntityModelTexturedMesh>(frame, vertices, indices);
        }

        void EntityModelSurface::addSkin(Assets::Texture* skin) {
            m_skins->addTexture(skin);
        }

        size_t EntityModelSurface::frameCount() const {
            return m_meshes.size();
        }

        size_t EntityModelSurface::skinCount() const {
            return m_skins->textureCount();
        }

        Assets::Texture* EntityModelSurface::skin(const std::string& name) const {
            return m_skins->textureByName(name);
        }

        Assets::Texture* EntityModelSurface::skin(const size_t index) const {
            return m_skins->textureByIndex(index);
        }

        std::unique_ptr<Renderer::TexturedIndexRangeRenderer> EntityModelSurface::buildRenderer(size_t skinIndex, size_t frameIndex) {
            if (skinIndex >= skinCount() || frameIndex >= frameCount() || m_meshes[frameIndex] == nullptr) {
                return nullptr;
            } else {
                auto* skin = this->skin(skinIndex);
                return m_meshes[frameIndex]->buildRenderer(skin);
            }
        }

        // EntityModel

        EntityModel::EntityModel(const std::string& name, PitchType pitchType) :
        m_name(name),
        m_prepared(false),
        m_pitchType(pitchType) {}

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
                m_frames.emplace_back(std::make_unique<EntityModelUnloadedFrame>(frameCount()));
            }
        }

        EntityModelLoadedFrame& EntityModel::loadFrame(const size_t frameIndex, const std::string& name, const vm::bbox3f& bounds) {
            if (frameIndex >= frameCount()) {
                throw AssetException("Frame index " + std::to_string(frameIndex) + " is out of bounds (frame count = " + std::to_string(frameCount()) + ")");
            }

            auto frame = std::make_unique<EntityModelLoadedFrame>(frameIndex, name, bounds, m_pitchType);
            auto& result = *frame;
            m_frames[frameIndex] = std::move(frame);
            return result;
        }

        EntityModelSurface& EntityModel::addSurface(const std::string& name) {
            m_surfaces.push_back(std::make_unique<EntityModelSurface>(name, frameCount()));
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

        std::vector<const EntityModelSurface*> EntityModel::surfaces() const {
            std::vector<const EntityModelSurface*> result;
            result.reserve(surfaceCount());
            for (const auto& surface : m_surfaces) {
                result.push_back(surface.get());
            }
            return result;
        }

        const EntityModelFrame* EntityModel::frame(const std::string& name) const {
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

        EntityModelSurface& EntityModel::surface(const size_t index) {
            if (index >= surfaceCount()) {
                throw std::out_of_range("Surface index is out of bounds");
            }
            return *m_surfaces[index];
        }

        const EntityModelSurface* EntityModel::surface(const std::string& name) const {
            for (const auto& surface : m_surfaces) {
                if (surface->name() == name) {
                    return surface.get();
                }
            }
            return nullptr;
        }
    }
}
