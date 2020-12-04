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

#pragma once

#include "Macros.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace Renderer {
        class FontManager;
        class ShaderManager;
        class VboManager;
    }

    namespace View {
        class GLContextManager {
        public:
            static std::string GLVendor;
            static std::string GLRenderer;
            static std::string GLVersion;
        private:
            bool m_initialized;

            std::string m_glVendor;
            std::string m_glRenderer;
            std::string m_glVersion;

            std::unique_ptr<Renderer::ShaderManager> m_shaderManager;
            std::unique_ptr<Renderer::VboManager> m_vboManager;
            std::unique_ptr<Renderer::FontManager> m_fontManager;
        public:
            GLContextManager();
            ~GLContextManager();

            bool initialized() const;
            bool initialize();

            Renderer::VboManager& vboManager();
            Renderer::FontManager& fontManager();
            Renderer::ShaderManager& shaderManager();

            deleteCopyAndMove(GLContextManager)
        };
    }
}

#endif /* defined(TrenchBroom_GLContextManager) */
