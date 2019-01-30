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
        class EntityModel {
        public:
            using Vertex = Renderer::VertexSpecs::P3T2::Vertex;
            using VertexList = Vertex::List;
            using Indices = Renderer::IndexRangeMap;
            using TexturedIndices = Renderer::TexturedIndexRangeMap;
        public:
            class Frame {
            private:
                String m_name;
                vm::bbox3f m_bounds;
            public:
                Frame(const String& name, const vm::bbox3f& bounds);

                const String& name() const;
                const vm::bbox3f& bounds() const;
            };

            class FrameData {
            private:
                VertexList m_vertices;
            protected:
                FrameData(const VertexList& vertices);
            public:
                virtual ~FrameData();
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(Assets::Texture* skin);
            private:
                virtual std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) = 0;
            };

            class IndexedFrameData : public FrameData {
            private:
                Indices m_indices;
            public:
                IndexedFrameData(const VertexList& vertices, const Indices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            class TexturedFrameData : public FrameData {
            private:
                TexturedIndices m_indices;
            public:
                TexturedFrameData(const VertexList& vertices, const TexturedIndices& indices);
            private:
                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            class Surface {
            private:
                String m_name;
                std::vector<std::unique_ptr<FrameData>> m_frames;
                std::unique_ptr<TextureCollection> m_skins;
            public:
                Surface(const String& name);

                void prepare(int minFilter, int magFilter);
                void setTextureMode(int minFilter, int magFilter);

                void addIndexedFrame(const VertexList& vertices, const Indices& indices);
                void addTexturedFrame(const VertexList& vertices, const TexturedIndices& indices);

                void addSkin(Assets::Texture* skin);

                size_t frameCount() const;
                size_t skinCount() const;

                std::unique_ptr<Renderer::TexturedIndexRangeRenderer> buildRenderer(size_t skinIndex, size_t frameIndex);
            };
        private:
            String m_name;
            bool m_prepared;
            std::vector<std::unique_ptr<Frame>> m_frames;
            std::vector<std::unique_ptr<Surface>> m_surfaces;
        public:
            explicit EntityModel(const String& name);

            Renderer::TexturedRenderer* buildRenderer(size_t skinIndex, size_t frameIndex) const;
            vm::bbox3f bounds(size_t skinIndex, size_t frameIndex) const;

            bool prepared() const;
            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);

            Frame& addFrame(const String& name, const vm::bbox3f& bounds);
            Surface& addSurface(const String& name);

            size_t frameCount() const;
            size_t surfaceCount() const;

            std::vector<const Frame*> frames() const;
            std::vector<const Surface*> surfaces() const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityModel) */
