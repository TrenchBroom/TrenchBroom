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

#pragma once

#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/VertexArray.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Renderer {
        class VboManager;
        class TextureRenderFunc;

        class TexturedRenderer {
        public:
            virtual ~TexturedRenderer();

            virtual bool empty() const = 0;

            virtual void prepare(VboManager& vboManager) = 0;
            virtual void render() = 0;
            virtual void render(TextureRenderFunc& func) = 0;
        };

        class TexturedIndexRangeRenderer : public TexturedRenderer {
        private:
            VertexArray m_vertexArray;
            TexturedIndexRangeMap m_indexRange;
        public:
            TexturedIndexRangeRenderer();
            TexturedIndexRangeRenderer(const VertexArray& vertexArray, const TexturedIndexRangeMap& indexRange);
            TexturedIndexRangeRenderer(const VertexArray& vertexArray, const Assets::Texture* texture, const IndexRangeMap& indexRange);
            ~TexturedIndexRangeRenderer() override;

            bool empty() const override;

            void prepare(VboManager& vboManager) override;
            void render() override;
            void render(TextureRenderFunc& func) override;
        };

        class MultiTexturedIndexRangeRenderer : public TexturedRenderer {
        private:
            std::vector<std::unique_ptr<TexturedIndexRangeRenderer>> m_renderers;
        public:
            MultiTexturedIndexRangeRenderer(std::vector<std::unique_ptr<TexturedIndexRangeRenderer>> renderers);
            ~MultiTexturedIndexRangeRenderer() override;

            bool empty() const override;

            void prepare(VboManager& vboManager) override;
            void render() override;
            void render(TextureRenderFunc& func) override;
        };
    }
}

#endif /* TexturedIndexRangeRenderer_h */
