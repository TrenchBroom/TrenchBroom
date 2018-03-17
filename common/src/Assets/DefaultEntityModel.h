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

#ifndef TRENCHBROOM_DEFAULTENTITYMODEL_H
#define TRENCHBROOM_DEFAULTENTITYMODEL_H

#include "Assets/EntityModel.h"
#include "Assets/TextureCollection.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;

        class DefaultEntityModel : public EntityModel {
        public:
            using Vertex = Renderer::VertexSpecs::P3T2::Vertex;
            using VertexList = Vertex::List;
            using Indices = Renderer::IndexRangeMap;
        private:
            class Frame {
            private:
                String m_name;
                BBox3f m_bounds;
                VertexList m_vertices;
                Indices m_indices;
            public:
                Frame(const String& name, const BBox3f& bounds, const VertexList& vertices, const Indices& indices);

                const BBox3f& bounds() const;
                Renderer::TexturedIndexRangeRenderer* buildRenderer(Assets::Texture* skin);
            };
        private:
            using FramePtr = std::unique_ptr<Frame>;
            using FrameList = std::vector<FramePtr>;
            using TextureCollectionPtr = std::unique_ptr<TextureCollection>;

            String m_name;
            FrameList m_frames;
            TextureCollectionPtr m_skins;
        public:
            DefaultEntityModel(const String& name);

            size_t frameCount() const override;
            size_t skinCount() const override;

            void addSkin(Assets::Texture* skin);
            void addFrame(const String& name, const VertexList& vertices, const Indices& indices);
        private:
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(size_t skinIndex, size_t frameIndex) const override;
            BBox3f doGetBounds(size_t skinIndex, size_t frameIndex) const override;
            void doPrepare(int minFilter, int magFilter) override;
            void doSetTextureMode(int minFilter, int magFilter) override;
        };
    }
}

#endif //TRENCHBROOM_DEFAULTENTITYMODEL_H
