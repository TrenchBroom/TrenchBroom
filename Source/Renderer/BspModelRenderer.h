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

#ifndef TrenchBroom_BspModelRenderer_h
#define TrenchBroom_BspModelRenderer_h

#include <GL/glew.h>
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/TextureVertexArray.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Bsp;
        class BspTexture;
        class Entity;
    }

    namespace Renderer {
        class Palette;
        class ShaderProgram;
        class TextureRenderer;
        class Vbo;
        class VboBlock;

        class BspModelRenderer : public EntityModelRenderer {
        private:
            typedef std::map<const Model::BspTexture*, TextureRenderer*> TextureCache;

            const Model::Bsp& m_bsp;

            const Palette& m_palette;
            TextureCache m_textures;

            Vbo& m_vbo;
            TextureVertexArrayList m_vertexArrays;
            
            void buildVertexArrays();
        public:
            BspModelRenderer(const Model::Bsp& bsp, Vbo& vbo, const Palette& palette);
            ~BspModelRenderer();
            
            void render(ShaderProgram& shaderProgram);
            
            const Vec3f& center() const;
            const BBoxf& bounds() const;
            BBoxf boundsAfterTransformation(const Mat4f& transformation) const;
        };
    }
}

#endif
