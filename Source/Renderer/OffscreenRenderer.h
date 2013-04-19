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

#ifndef __TrenchBroom__OffscreenRenderer__
#define __TrenchBroom__OffscreenRenderer__

#include "GL/glew.h"

#include <wx/image.h>

namespace TrenchBroom {
    namespace GL {
        class Capabilities;
    }

    namespace Renderer {
        class OffscreenRenderer {
        private:
            GLuint m_framebufferId;
            GLuint m_colorbufferId;
            GLuint m_depthbufferId;

            bool m_valid;
            unsigned int m_width;
            unsigned int m_height;

            bool m_multisample;
            GLint m_samples;

            OffscreenRenderer* m_readBuffers;
        public:
            OffscreenRenderer(bool multisample, GLint samples = 0);
            ~OffscreenRenderer();

            inline void setDimensions(unsigned int width, unsigned int height) {
                if (width == m_width && height == m_height)
                    return;
                m_width = width;
                m_height = height;
                m_valid = false;
            }

            void preRender();
            void postRender();

            wxImage* getImage();
        };
    }
}

#endif /* defined(__TrenchBroom__OffscreenRenderer__) */
