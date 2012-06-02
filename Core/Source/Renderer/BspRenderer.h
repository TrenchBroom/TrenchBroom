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

#include <string>
#include <map>
#include <vector>

#include "Renderer/EntityRenderer.h"
#include "GL/GLee.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Bsp;
            class Palette;
            class Texture;
        }

        class Entity;
    }

    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VboBlock;

        class BspRenderer : public EntityRenderer {
        private:
            typedef map<string, Model::Assets::Texture*> TextureCache;
            typedef vector<GLint> IntBuffer;
            typedef pair<IntBuffer, IntBuffer> InfoBuffer;
            typedef map<Model::Assets::Texture*, InfoBuffer> TextureVertexInfo;

            Model::Assets::Bsp& m_bsp;
            Vbo& m_vbo;
            Model::Assets::Palette& m_palette;
            VboBlock* m_vboBlock;
            TextureCache m_textures;
            TextureVertexInfo m_vertexInfos;
        public:
            BspRenderer(Model::Assets::Bsp& bsp, Vbo& vbo, Model::Assets::Palette& palette);
            ~BspRenderer();
            void render();
            const Vec3f& center();
            const BBox& bounds();
        };
    }
}

#endif
