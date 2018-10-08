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

namespace TrenchBroom {
    namespace Renderer {
        class TexturedIndexRangeRenderer;
    }
    
    namespace Assets {
        class EntityModel {
        public:
            using Vertex = Renderer::VertexSpecs::P3T2::Vertex;
            using VertexList = Vertex::List;
            using Indices = Renderer::IndexRangeMap;
            using TexturedIndices = Renderer::TexturedIndexRangeMap;
        private:
            class Frame {
            private:
                String m_name;
                vm::bbox3f m_bounds;
                VertexList m_vertices;
            protected:
                Frame(const String& name, const vm::bbox3f& bounds, const VertexList& vertices);
            public:
                virtual ~Frame();

                const vm::bbox3f& bounds() const;
                Renderer::TexturedIndexRangeRenderer* buildRenderer(Assets::Texture* skin);
            private:
                virtual Renderer::TexturedIndexRangeRenderer* doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) = 0;
            };

            class IndexedFrame : public Frame {
            private:
                Indices m_indices;
            public:
                IndexedFrame(const String& name, const vm::bbox3f& bounds, const VertexList& vertices, const Indices& indices);
            private:
                Renderer::TexturedIndexRangeRenderer* doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };

            class TexturedFrame : public Frame {
            private:
                TexturedIndices m_indices;
            public:
                TexturedFrame(const String& name, const vm::bbox3f& bounds, const VertexList& vertices, const TexturedIndices& indices);
            private:
                Renderer::TexturedIndexRangeRenderer* doBuildRenderer(Assets::Texture* skin, const Renderer::VertexArray& vertices) override;
            };
        private:
            using FramePtr = std::unique_ptr<Frame>;
            using FrameList = std::vector<FramePtr>;
            using TextureCollectionPtr = std::unique_ptr<TextureCollection>;

            String m_name;
            FrameList m_frames;
            TextureCollectionPtr m_skins;
            bool m_prepared;
        public:
            EntityModel(const String& name);

            Renderer::TexturedIndexRangeRenderer* buildRenderer(size_t skinIndex, size_t frameIndex) const;
            vm::bbox3f bounds(size_t skinIndex, size_t frameIndex) const;

            size_t frameCount() const;
            size_t skinCount() const;

            Assets::Texture* skin(size_t index) const;

            bool prepared() const;
            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);
        public:
            void addSkin(Assets::Texture* skin);
            void addFrame(const String& name, const VertexList& vertices, const Indices& indices);
            void addFrame(const String& name, const VertexList& vertices, const TexturedIndices& indices);
        };
    }
}

#endif /* defined(TrenchBroom_EntityModel) */
