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

#ifndef TrenchBroom_AliasModelRenderer_h
#define TrenchBroom_AliasModelRenderer_h

#include "Renderer/EntityModelRenderer.h"
#include "Renderer/TextureRendererTypes.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Model {
        class Alias;
        class Entity;
    }

    namespace Renderer {
        class Palette;
        class RenderContext;
        class ShaderProgram;
        class Vbo;
        

        class AliasModelRenderer : public EntityModelRenderer {
        private:
            const Model::Alias& m_alias;
            unsigned int m_skinIndex;

            const Palette& m_palette;
            TextureRendererPtr m_texture;

            Vbo& m_vbo;
            VertexArray* m_vertexArray;
        public:
            AliasModelRenderer(const Model::Alias& alias, int unsigned skinIndex, Vbo& vbo, const Palette& palette);
            ~AliasModelRenderer();
            
            void render(ShaderProgram& shaderProgram);
            
            const Vec3f& center() const;
            const BBox& bounds() const;
            BBox boundsAfterTransformation(const Mat4f& transformation) const;
        };
    }
}

#endif
