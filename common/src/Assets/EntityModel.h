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

#ifndef TrenchBroom_EntityModel
#define TrenchBroom_EntityModel

#include "Assets/TextureCollection.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMap.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

template <typename T, size_t S, typename U>
class AABBTree;

namespace TrenchBroom {
    namespace Renderer {
        class TexturedIndexRangeRenderer;
        class TexturedRenderer;
    }

    namespace Assets {
        /**
         * One frame of the model. Since frames are loaded on demand, each frame has two possible states: loaded
         * and unloaded. These states are modeled as subclasses of this class.
         */
        class EntityModelFrame {
        private:
            size_t m_index;
        public:
            /**
             * Creates a new frame with the given index.
             *
             * @param index the index of this frame
             */
            explicit EntityModelFrame(size_t index);

            virtual ~EntityModelFrame();

            /**
             * Indicates whether this frame is already loaded.
             *
             * @return true if this frame is loaded and false otherwise
             */
            virtual bool loaded() const = 0;

            /**
             * Returns the index of this frame.
             *
             * @return the index
             */
            size_t index() const;

            /**
             * Returns this frame's name.
             *
             * @return the name
             */
            virtual const std::string& name() const = 0;


            /**
             * Returns this frame's bounding box.
             *
             * @return the bounding box
             */
            virtual const vm::bbox3f& bounds() const = 0;

            /**
             * Intersects this frame with the given ray and returns the point of intersection.
             *
             * @param ray the ray to intersect
             * @return the distance to the point of intersection or NaN if the given ray does not intersect this frame
             */
            virtual float intersect(const vm::ray3f& ray) const = 0;
        };

        /**
         * Manages all data necessary to render an entity model. Each model can have multiple frames, and
         * multiple surfaces. Each surface represents an independent mesh of primitives such as triangles, and
         * the corresponding textures. Every surface has a separate mesh for each frame of the model.
         */
        class EntityModel {
        public:
            using Vertex = Renderer::GLVertexTypes::P3T2::Vertex;
            using VertexList = Vertex::List;
            using Indices = Renderer::IndexRangeMap;
            using TexturedIndices = Renderer::TexturedIndexRangeMap;
        public:
            class LoadedFrame : public EntityModelFrame {
            private:
                std::string m_name;
                vm::bbox3f m_bounds;

                // For hit testing
                using Triangle = std::array<vm::vec3f, 3>;
                std::vector<Triangle> m_tris;
                using TriNum = size_t;
                using SpacialTree = AABBTree<float, 3, TriNum>;
                std::unique_ptr<SpacialTree> m_spacialTree;
            public:
                /**
                 * Creates a new frame with the given index, name and bounds.
                 *
                 * @param index the index of this frame
                 * @param name the frame name
                 * @param bounds the bounding box of the frame
                 */
                LoadedFrame(size_t index, const std::string& name, const vm::bbox3f& bounds);

                bool loaded() const override;
                const std::string& name() const override;
                const vm::bbox3f& bounds() const override;
                float intersect(const vm::ray3f& ray) const override;

                /**
                 * Adds the given primitives to the spacial tree for this frame.
                 *
                 * @param vertices the vertices
                 * @param primType the primitive type
                 * @param index the index of the first primitive's first vertex in the given vertex array
                 * @param count the number of vertices that make up the primitive(s)
                 */
                void addToSpacialTree(const VertexList& vertices, PrimType primType, size_t index, size_t count);
            };

            class UnloadedFrame : public EntityModelFrame {
            public:
                /**
                 * Creates a new frame with the given index.
                 *
                 * @param index the index of this frame
                 */
                explicit UnloadedFrame(size_t index);

                bool loaded() const override;
                const std::string& name() const override;
                const vm::bbox3f& bounds() const override;
                float intersect(const vm::ray3f& ray) const override;
            };

            /**
             * The mesh associated with a frame and a surface.
             */
            class Mesh {
            private:
                VertexList m_vertices;
            protected:
                /**
                 * Creates a new frame mesh that uses the given vertices.
                 *
                 * @param vertices the vertices
                 */
                explicit Mesh(const VertexList& vertices);
            public:
                virtual ~Mesh();

                /**
                 * Returns a renderer that renders this mesh with the given texture.
                 *
                 * @param skin the texture to use when rendering the mesh
                 * @return the renderer
                 */
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(Assets::Texture* skin);
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

            /**
             * A model frame mesh for indexed rendering. Stores vertices and vertex indices.
             */
            class IndexedMesh : public Mesh {
            private:
                Indices m_indices;
            public:
                /**
                 * Creates a new frame mesh with the given vertices and indices.
                 *
                 * @param frame the frame to which this mesh belongs
                 * @param vertices the vertices
                 * @param indices the indices
                 */
                IndexedMesh(LoadedFrame& frame, const VertexList& vertices, const Indices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            /**
             * A model frame mesh for per texture indexed rendering. Stores vertices and per texture indices.
             */
            class TexturedMesh : public Mesh {
            private:
                TexturedIndices m_indices;
            public:
                /**
                 * Creates a new frame mesh with the given vertices and per texture indices.
                 *
                 * @param frame the frame to which this mesh belongs
                 * @param vertices the vertices
                 * @param indices the per texture indices
                 */
                TexturedMesh(LoadedFrame& frame, const VertexList& vertices, const TexturedIndices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            /**
             * A model surface represents an individual part of a model. MDL and MD2 models use only one surface, while
             * more complex model formats such as MD3 contain multiple surfaces with one skin per surface.
             *
             * Each surface contains per frame meshes. The number of per frame meshes should match the number of frames
             * in the model.
             */
            class Surface {
            private:
                std::string m_name;
                std::vector<std::unique_ptr<Mesh>> m_meshes;
                std::unique_ptr<TextureCollection> m_skins;
            public:
                /**
                 * Creates a new surface with the given name.
                 *
                 * @param name the surface's name
                 * @param frameCount the number of frames
                 */
                explicit Surface(const std::string& name, size_t frameCount);


                /**
                 * Returns the name of this surface.
                 *
                 * @return the name of this surface
                 */
                const std::string& name() const;

                /**
                 * Prepares the skin textures of this surface for rendering.
                 *
                 * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
                 * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
                 */
                void prepare(int minFilter, int magFilter);

                /**
                 * Sets the minification and magnification filters for the skin textures of this surface.
                 *
                 * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
                 * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
                 */
                void setTextureMode(int minFilter, int magFilter);

                /**
                 * Adds a new mesh to this surface.
                 *
                 * @param frame the frame which the mesh belongs to
                 * @param vertices the mesh vertices
                 * @param indices the vertex indices
                 */
                void addIndexedMesh(LoadedFrame& frame, const VertexList& vertices, const Indices& indices);

                /**
                 * Adds a new multitextured mesh to this surface.
                 *
                 * @param frame the frame which the mesh belongs to
                 * @param vertices the mesh vertices
                 * @param indices the per texture vertex indices
                 */
                void addTexturedMesh(LoadedFrame& frame, const VertexList& vertices, const TexturedIndices& indices);

                /**
                 * Adds the given texture as a skin to this surface.
                 *
                 *@param skin the skin to add
                 */
                void addSkin(Assets::Texture* skin);

                /**
                 * Returns the number of frame meshes in this surface, should match the model's frame count.
                 *
                 * @return the number of frame meshes
                 */
                size_t frameCount() const;

                /**
                 * Returns the number of skins of this surface.
                 *
                 * @return the number of skins
                 */
                size_t skinCount() const;

                /**
                 * Returns the skin with the given name.
                 *
                 * @param name the name of the skin to find
                 * @return the skin with the given name, or null if no such skin was found
                 */
                Assets::Texture* skin(const std::string& name) const;

                /**
                 * Returns the skin with the given index.
                 *
                 * @param index the index of the skin to find
                 * @return the skin with the given index, or null if the index is out of bounds
                 */
                Assets::Texture* skin(size_t index) const;

                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(size_t skinIndex, size_t frameIndex);
            };
        private:
            std::string m_name;
            bool m_prepared;
            std::vector<std::unique_ptr<EntityModelFrame>> m_frames;
            std::vector<std::unique_ptr<Surface>> m_surfaces;
        public:
            /**
             * Creates a new entity model with the given name.

             * @param name the name of the model
             */
            explicit EntityModel(const std::string& name);

            /**
             * Creates a renderer to render the given frame of the model using the skin with the given index.
             *
             * @param skinIndex the index of the skin to use
             * @param frameIndex the index of the frame to render
             * @return the renderer
             */
            std::unique_ptr<Renderer::TexturedRenderer> buildRenderer(size_t skinIndex, size_t frameIndex) const;

            /**
             * Returns the bounds of the given frame of this model.
             *
             * @param frameIndex the index of the frame
             * @return the bounds of the frame
             */
            vm::bbox3f bounds(size_t frameIndex) const;

            /**
             * Indicates whether or not this model has been prepared for rendering.
             *
             * @return true if this model has been prepared and false otherwise
             */
            bool prepared() const;

            /**
             * Prepares this model for rendering by uploading its skin textures.
             *
             * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
             * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
             */
            void prepare(int minFilter, int magFilter);

            /**
             * Sets the minification and magnification filters for the skin textures of this model.
             *
             * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
             * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
             */
            void setTextureMode(int minFilter, int magFilter);

            /**
             * Adds the given number of frames to this model.
             *
             * @param frameCount the number of frames to add
             */
            void addFrames(size_t frameCount = 1);

            /**
             * Adds a frame with the given name and bounds.
             *
             * @param frameIndex the frame's index
             * @param name the frame name
             * @param bounds the frame bounds
             * @return the newly added frame
             *
             * @throws AssetException if the given frame index is out of bounds
             */
            LoadedFrame& loadFrame(size_t frameIndex, const std::string& name, const vm::bbox3f& bounds);

            /**
             * Adds a surface with the given name.
             *
             * @param name the surface name
             * @return the newly added surface
             */
            Surface& addSurface(const std::string& name);

            /**
             * Returns the number of frames of this model.
             *
             * @return the number of frames
             */
            size_t frameCount() const;

            /**
             * Returns the number of surfaces of this model.
             *
             * @return the number of surfaces
             */
            size_t surfaceCount() const;

            /**
             * Returns all frames of this model.
             *
             * @return the frames
             */
            std::vector<const EntityModelFrame*> frames() const;

            /**
             * Returns all frames of this model.
             *
             * @return the frames
             */
            std::vector<EntityModelFrame*> frames();

            /**
             * Returns all surfaces of this model.
             *
             * @return the surfaces
             */
            std::vector<const Surface*> surfaces() const;

            /**
             * Returns the frame with the given name.
             *
             * @param name the name of the frame to find
             * @return the frame with the given name or null if no such frame was found
             */
            const EntityModelFrame* frame(const std::string& name) const;

            /**
             * Returns the frame with the given index.
             *
             * @param index the index of the frame
             * @return the frame with the given index or null if the index is out of bounds
             */
            const EntityModelFrame* frame(size_t index) const;

            /**
             * Returns the surface with the given index.
             *
             * @param index the index of the surface to return
             * @return the surface with the given index
             * @throw std::out_of_range if the given index is out of bounds
             */
            Surface& surface(size_t index);

            /**
             * Returns the surface with the given name.
             *
             * @param name the name of the surface to find
             * @return the surface with the given name or null if no such surface was found
             */
            const Surface* surface(const std::string& name) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityModel) */
