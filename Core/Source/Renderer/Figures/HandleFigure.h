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

#ifndef TrenchBroom_BrushVertexFigure_h
#define TrenchBroom_BrushVertexFigure_h

#include "Renderer/Figures/Figure.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;
        
        class HandleFigure : public Figure {
        protected:
            VboBlock* m_vboBlock;
            bool m_valid;
            Vec3fList m_positions;
            Vec4f m_color;
            Vec4f m_hiddenColor;
        public:
            HandleFigure() : m_valid(false), m_vboBlock(NULL), m_color(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), m_hiddenColor(1.0f, 1.0f, 1.0f, 0.5f) {}
            ~HandleFigure();
            
            void setPositions(const Vec3fList& positions);
            void setColor(const Vec4f& color);
            void setHiddenColor(const Vec4f& hiddenColor);
            virtual void render(RenderContext& context, Vbo& vbo);
        };
    }
}

#endif
