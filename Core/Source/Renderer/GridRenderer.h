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

#include <vector>
#include "GL/GLee.h"

namespace TrenchBroom {
    namespace Controller {
        class Grid;
    }

    namespace Renderer {
        class GridRenderer {
        private:
            float m_alpha;
            std::vector<GLuint> m_textures;
            void clear();
        public:
            GridRenderer(float alpha) : m_alpha(alpha) {}
            ~GridRenderer();
            void setAlpha(float alpha);
            void activate(const Controller::Grid& grid);
            void deactivate();
        };
    }
}

#endif
