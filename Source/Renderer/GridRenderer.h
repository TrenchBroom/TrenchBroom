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

#ifndef TrenchBroom_GridRenderer_h
#define TrenchBroom_GridRenderer_h

#include "Utility/Color.h"
#include "Utility/GLee.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class Grid;
    }

    namespace Renderer {
        class GridRenderer {
        private:
            bool m_valid;
            Color m_color;
            std::vector<GLuint> m_textures;
            void clear();
        public:
            GridRenderer() : m_color(Color(1.0f, 1.0f, 1.0f, 1.0f)), m_valid(false) {}
            ~GridRenderer();

            inline void setColor(const Color& color) {
                if (m_color == color)
                    return;
                m_color = color;
                m_valid = false;
            }
            
            void activate(const Controller::Grid& grid);
            void deactivate();
        };
    }
}

#endif
