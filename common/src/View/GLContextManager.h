/*
 Copyright (C) 2010-2017 Kristian Duske

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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_GLContextManager
#define TrenchBroom_GLContextManager

#include "Macros.h"
#include "StringUtils.h"

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class ShaderManager;
        class Vbo;
    }

    namespace View {
        class GLContextManager {
        public:
            static String GLVendor;
            static String GLRenderer;
            static String GLVersion;
        private:
            bool m_initialized;

            String m_glVendor;
            String m_glRenderer;
            String m_glVersion;

            std::unique_ptr<Renderer::Vbo> m_vertexVbo;
            std::unique_ptr<Renderer::Vbo> m_indexVbo;
            std::unique_ptr<Renderer::FontManager> m_fontManager;
            std::unique_ptr<Renderer::ShaderManager> m_shaderManager;
        public:
            GLContextManager();
            ~GLContextManager();

            bool initialized() const;
            bool initialize();

            Renderer::Vbo& vertexVbo();
            Renderer::Vbo& indexVbo();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();

            deleteCopyAndMove(GLContextManager)
        };
    }
}

#endif /* defined(TrenchBroom_GLContextManager) */
