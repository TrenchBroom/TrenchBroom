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

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class TexturedIndexRangeRenderer;
        class TexturedRenderer;
    }

    namespace Assets {

        /**
         Manages all data necessary to render an entity model. Each model can have multiple frames, and
         multiple surfaces. Each surface represents an independent mesh of primitives such as triangles, and
         the corresponding textures. Every surface has a separate mesh for each frame of the model.
         */
        class EntityModel {
        public:
            using Vertex = Renderer::GLVertexTypes::P3T2::Vertex;
            using VertexList = Vertex::List;
            using Indices = Renderer::IndexRangeMap;
            using TexturedIndices = Renderer::TexturedIndexRangeMap;
        public:
            /**
             One frame of the model.
             */
            class Frame {
            private:
                String m_name;
                vm::bbox3f m_bounds;
            public:
                /**
                 Creates a new frame with the given name and bounds.

                 @param name the frame name
                 @param bounds the bounding box of the frame
                 */
                Frame(const String& name, const vm::bbox3f& bounds);

                /**
                 Returns this frame's name.

                 @return the name
                 */
                const String& name() const;


                /**
                 Returns this frame's bounding box.

                 @return the bounding box
                 */
                const vm::bbox3f& bounds() const;
            };

            /**
             The mesh associated with a frame and a surface.
             */
            class Mesh {
            private:
                VertexList m_vertices;
            protected:
                /**
                 Creates a new frame mesh that uses the given vertices.

                 @param vertices the vertices
                 */
                Mesh(const VertexList& vertices);
            public:
                virtual ~Mesh();

                /**
                 Returns a renderer that renders this mesh with the given texture.

                 @param skin the texture to use when rendering the mesh
                 @return the renderer
                 */
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(Assets::Texture* skin);
            private:

                /**
                 Creates and returns the actual mesh renderer

                 @param skin the skin to use when rendering the mesh
                 @param vertices the vertices associated with this mesh
                 @return the renderer
                 */
                virtual std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) = 0;
            };

            /**
             A model frame mesh for indexed rendering. Stores vertices and vertex indices.
             */
            class IndexedMesh : public Mesh {
            private:
                Indices m_indices;
            public:
                /**
                 Creates a new frame mesh with the given vertices and indices.

                 @param vertices the vertices
                 @param indices the indices
                 */
                IndexedMesh(const VertexList& vertices, const Indices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            /**
             A model frame mesh for per texture indexed rendering. Stores vertices and per texture indices.
             */
            class TexturedMesh : public Mesh {
            private:
                TexturedIndices m_indices;
            public:
                /**
                 Creates a new frame mesh with the given vertices and per texture indices.

                 @param vertices the vertices
                 @param indices the per texture indices
                 */
                TexturedMesh(const VertexList& vertices, const TexturedIndices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            /**
             A model surface represents an individual part of a model. MDL and MD2 models use only one surface, while
             more complex model formats such as MD3 contain multiple surfaces with one skin per surface.

             Each surface contains per frame meshes. The number of per frame meshes should match the number of frames
             in the model.
             */
            class Surface {
            private:
                String m_name;
                std::vector<std::unique_ptr<Mesh>> m_meshes;
                std::unique_ptr<TextureCollection> m_skins;
            public:
                /**
                 Creates a new surface with the given name.

                 @param name the surface's name
                 */
                Surface(const String& name);


                /**
                 Returns the name of this surface.

                 @return the name of this surface
                 */
                const String& name() const;

                /**
                 Prepares the skin textures of this surface for rendering.

                 @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
                 @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
                 */
                void prepare(int minFilter, int magFilter);

                /**
                 Sets the minification and magnification filters for the skin textures of this surface.

                 @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
                 @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
                 */
                void setTextureMode(int minFilter, int magFilter);

                /**
                 Adds a new mesh to this surface.

                 @param vertices the mesh vertices
                 @param indices the vertex indices
                 */
                void addIndexedMesh(const VertexList& vertices, const Indices& indices);

                /**
                 Adds a new multitextured mesh to this surface.

                 @param vertices the mesh vertices
                 @param indices the per texture vertex indices
                 */
                void addTexturedMesh(const VertexList& vertices, const TexturedIndices& indices);

                /**
                 Adds the given texture as a skin to this surface.

                 @param skin the skin to add
                 */
                void addSkin(Assets::Texture* skin);

                /**
                 Returns the number of frame meshes in this surface, should match the model's frame count.

                 @return the number of frame meshes
                 */
                size_t frameCount() const;

                /**
                 Returns the number of skins of this surface.

                 @return the number of skins
                 */
                size_t skinCount() const;

                /**
                 Returns the skin with the given name.

                 @param name the name of the skin to find
                 @return the skin with the given name, or null if no such skin was found
                 */
                const Assets::Texture* skin(const String& name) const;

                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(size_t skinIndex, size_t frameIndex);
            };
        private:
            String m_name;
            bool m_prepared;
            std::vector<std::unique_ptr<Frame>> m_frames;
            std::vector<std::unique_ptr<Surface>> m_surfaces;
        public:
            /**
             Creates a new entity model with the given name.

             @param name the name of the model
             */
            explicit EntityModel(const String& name);

            /**
             Creates a renderer to render the given frame of the model using the skin with the given index.

             @param skinIndex the index of the skin to use
             @param frameIndex the index of the frame to render
             @return the renderer
             */
            Renderer::TexturedRenderer* buildRenderer(size_t skinIndex, size_t frameIndex) const;

            /**
             Returns the bounds of the given frame of this model.

             @param skinIndex the skin, unused
             @param frameIndex the index of the frame
             @return the bounds of the frame
             */
            vm::bbox3f bounds(size_t skinIndex, size_t frameIndex) const;

            /**
             Indicates whether or not this model has been prepared for rendering.

             @return true if this model has been prepared and false otherwise
             */
            bool prepared() const;

            /**
             Prepares this model for rendering by uploading its skin textures.

             @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
             @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
             */
            void prepare(int minFilter, int magFilter);

            /**
             Sets the minification and magnification filters for the skin textures of this model.

             @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
             @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
             */
            void setTextureMode(int minFilter, int magFilter);

            /**
             Adds a frame with the given name and bounds.

             @param name the frame name
             @param bounds the frame bounds
             @return the newly added frame
             */
            Frame& addFrame(const String& name, const vm::bbox3f& bounds);

            /**
             Adds a surface with the given name.

             @param name the surface name
             @return the newly added surface
             */
            Surface& addSurface(const String& name);

            /**
             Returns the number of frames of this model.

             @return the number of frames
             */
            size_t frameCount() const;

            /**
             Returns the number of surfaces of this model.

             @return the number of surfaces
             */
            size_t surfaceCount() const;

            /**
             Returns all frames of this model.

             @return the frames
             */
            std::vector<const Frame*> frames() const;

            /**
             Returns all surfaces of this model.

             @return the surfaces
             */
            std::vector<const Surface*> surfaces() const;

            /**
             Returns the frame with the given name.

             @param name the name of the frame to find
             @return the frame with the given name or null if no such frame was found
             */
            const Frame* frame(const String& name) const;

            /**
             Returns the surface with the given name.

             @param name the name of the surface to find
             @return the surface with the given name or null if no such surface was found
             */
            const Surface* surface(const String& name) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityModel) */
