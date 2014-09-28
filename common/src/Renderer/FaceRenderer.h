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
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class ActiveShader;
        class RenderContext;
        class Vbo;
        
        class FaceRenderer {
        public:
            struct Config {
                float alpha;
                bool grayscale;
                bool tinted;
                Color tintColor;
                Config();
            };
        private:
            Vbo::Ptr m_vbo;
            MeshRenderer m_meshRenderer;
            Color m_faceColor;
            bool m_prepared;
        public:
            FaceRenderer();
            FaceRenderer(Model::BrushFace::Mesh& mesh, const Color& faceColor);
            FaceRenderer(const FaceRenderer& other);
            FaceRenderer& operator= (FaceRenderer other);
            
            friend void swap(FaceRenderer& left, FaceRenderer& right);

            void render(RenderContext& context, const Config& config);
        private:
            void render(RenderContext& context, float alpha, bool grayscale, const Color* tintColor);
            void prepare();
        };

        void swap(FaceRenderer& left, FaceRenderer& right);
    }
}

#endif /* defined(__TrenchBroom__FaceRenderer__) */
