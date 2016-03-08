/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TriangleMeshRenderer.h"
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Renderer {
        void TexturedTriangleMeshRenderer::DefaultMeshFunc::before(const Assets::Texture* const & texture) const {
            if (texture != NULL)
                texture->activate();
        }
        
        void TexturedTriangleMeshRenderer::DefaultMeshFunc::after(const Assets::Texture* const & texture) const {
            if (texture != NULL)
                texture->deactivate();
        }
        
        TexturedTriangleMeshRenderer::TexturedTriangleMeshRenderer() :
        TriangleMeshRendererBase() {}

        void TexturedTriangleMeshRenderer::render() {
            render(DefaultMeshFunc());
        }
        
        void TexturedTriangleMeshRenderer::render(const MeshFuncBase& func) {
            TriangleMeshRendererBase<const Assets::Texture*>::performRender(func);
        }

        void SimpleTriangleMeshRenderer::NopMeshFunc::before(const int& key) const {}
        void SimpleTriangleMeshRenderer::NopMeshFunc::after(const int& key) const {}

        SimpleTriangleMeshRenderer::SimpleTriangleMeshRenderer() :
        TriangleMeshRendererBase() {}
        
        void SimpleTriangleMeshRenderer::render() {
            render(NopMeshFunc());
        }

        void SimpleTriangleMeshRenderer::render(const MeshFuncBase& func) {
            TriangleMeshRendererBase<int>::performRender(func);
        }
    }
}
