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

#include "Model/TextureTypes.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Model {
        class Alias;
        class Entity;
        class Palette;
    }

    namespace Renderer {
        class RenderContext;
        class ShaderProgram;
        class Vbo;
        

        class AliasRenderer : public EntityRenderer {
        private:
            const Model::Alias& m_alias;
            unsigned int m_skinIndex;

            const Model::Palette& m_palette;
            Model::TexturePtr m_texture;

            Vbo& m_vbo;
            VertexArrayPtr m_vertexArray;
        public:
            AliasRenderer(const Model::Alias& alias, int unsigned skinIndex, Vbo& vbo, const Model::Palette& palette);
            
            void render(ShaderProgram& shaderProgram);
            
            const Vec3f& center() const;
            const BBox& bounds() const;
        };
    }
}

#endif
