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

#ifndef TrenchBroom_BspRenderer_h
#define TrenchBroom_BspRenderer_h

#include "Renderer/EntityRenderer.h"
#include "Utility/GLee.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Bsp;
        class Entity;
        class Palette;
        class Texture;
    }

    namespace Renderer {
        class Vbo;
        class VboBlock;

        class BspRenderer : public EntityRenderer {
        private:
            typedef std::map<String, Model::Texture*> TextureCache;
            typedef std::vector<GLint> IntBuffer;
            typedef std::pair<IntBuffer, IntBuffer> InfoBuffer;
            typedef std::map<Model::Texture*, InfoBuffer> TextureVertexInfo;

            const Model::Bsp& m_bsp;
            const Model::Palette& m_palette;

            Vbo& m_vbo;
            VboBlock* m_vboBlock;
            TextureCache m_textures;
            TextureVertexInfo m_vertexInfos;
        public:
            BspRenderer(const Model::Bsp& bsp, Vbo& vbo, const Model::Palette& palette);
            ~BspRenderer();
            
            void render();
            
            const Vec3f& center() const;
            const BBox& bounds() const;
        };
    }
}

#endif
