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

#include "GridRenderer.h"
#include <cassert>
#include "Texture.h"
#include "Grid.h"

namespace TrenchBroom {
    namespace Renderer {
        void GridRenderer::clear() {
            for (int i = 0; i < m_textures.size(); i++)
                if (m_textures[i] != -1)
                    glDeleteTextures(1, &m_textures[i]);
            m_textures.clear();
        }
        
        GridRenderer::~GridRenderer() {
            clear();
        }
        
        void GridRenderer::setAlpha(float alpha) {
            if (m_alpha == alpha) return;
            m_alpha = alpha;
            clear();
        }
        
        void GridRenderer::activate(const Controller::Grid& grid) {
            int index = grid.size();
            if (index >= m_textures.size()) {
                m_textures.resize(index + 1);
                for (int i = index; i < m_textures.size(); i++)
                    m_textures[i] = -1;
            }

            GLuint textureId = m_textures[index];
            if (textureId == -1) {
                glGenTextures(1, &textureId);
                int dim = grid.actualSize();
                if (dim < 4) dim = 4;
                int texSize = 1 << 8;
                unsigned char pixel[texSize * texSize * 4];
                for (int y = 0; y < texSize; y++) {
                    for (int x = 0; x < texSize; x++) {
                        int i = (y * texSize + x) * 4;
                        if ((x % dim) == 0 || (y % dim) == 0) {
                            pixel[i + 0] = 0xFF;
                            pixel[i + 1] = 0xFF;
                            pixel[i + 2] = 0xFF;
                            pixel[i + 3] = 0xFF * m_alpha;
                        } else {
                            pixel[i + 0] = 0x00;
                            pixel[i + 1] = 0x00;
                            pixel[i + 2] = 0x00;
                            pixel[i + 3] = 0x00;
                        }
                    }
                }

                glBindTexture(GL_TEXTURE_2D,textureId);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
                m_textures[index] = textureId;
            }
            
            glBindTexture(GL_TEXTURE_2D, textureId);
        }
        
        void GridRenderer::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

