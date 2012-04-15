/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_AliasRenderer_h
#define TrenchBroom_AliasRenderer_h

#include "Renderer/EntityRenderer.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Alias;
            class Texture;
            class Palette;
        }

        class Entity;
    }

    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VboBlock;

        class AliasRenderer : public EntityRenderer {
        private:
            Model::Assets::Alias& m_alias;
            int m_skinIndex;
            Vbo& m_vbo;
            Model::Assets::Palette& m_palette;
            VboBlock* m_vboBlock;
            Model::Assets::Texture* m_texture;
            int m_triangleCount;
        public:
            AliasRenderer(Model::Assets::Alias& alias, int skinIndex, Vbo& vbo, Model::Assets::Palette& palette);
            ~AliasRenderer();
            void render(RenderContext& context, Model::Entity& entity);
            void render(RenderContext& context, const Vec3f& position, float angle);
            const Vec3f& center();
            const BBox& bounds();
            const BBox& maxBounds();
        };
    }
}

#endif
