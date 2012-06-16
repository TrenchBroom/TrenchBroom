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

#ifndef TrenchBroom_BrushFigure_h
#define TrenchBroom_BrushFigure_h

#include "Model/Map/BrushTypes.h"
#include "Renderer/Figures/Figure.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
        }
    }
    
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VboBlock;
        
        class BrushFigure : public Figure {
        protected:
            Model::BrushList m_brushes;
            Model::Assets::Texture* m_dummyTexture;
            Vbo* m_vbo;
            VboBlock* m_edgeBlock;
            VboBlock* m_faceBlock;
            unsigned int m_edgeVertexCount;
        public:
            BrushFigure(const Model::BrushList& brushes);
            ~BrushFigure();
            virtual void render(RenderContext& context);
        };
    }
}

#endif
