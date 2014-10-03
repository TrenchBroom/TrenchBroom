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

#ifndef __TrenchBroom__FaceRenderer__
#define __TrenchBroom__FaceRenderer__

#include "Color.h"
#include "Assets/AssetTypes.h"
#include "Model/BrushFace.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderBatch;
        class RenderContext;
        class Vbo;
        
        class FaceRenderer : public Renderable {
        private:
            MeshRenderer m_meshRenderer;
            Color m_faceColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            float m_alpha;
            bool m_prepared;
        public:
            FaceRenderer();
            FaceRenderer(Model::BrushFace::Mesh& mesh, const Color& faceColor);
            FaceRenderer(const FaceRenderer& other);
            FaceRenderer& operator= (FaceRenderer other);
            
            friend void swap(FaceRenderer& left, FaceRenderer& right);

            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& color);
            void setAlpha(float alpha);
            
            void render(RenderBatch& renderBatch);
        private:
            void doPrepare(Vbo& vbo);
            void doRender(RenderContext& context);
        };

        void swap(FaceRenderer& left, FaceRenderer& right);
    }
}

#endif /* defined(__TrenchBroom__FaceRenderer__) */
