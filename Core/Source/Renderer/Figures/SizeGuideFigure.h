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

#ifndef TrenchBroom_SizeGuideFigure_h
#define TrenchBroom_SizeGuideFigure_h

#include "Renderer/FontManager.h"
#include "Renderer/Figures/Figure.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;
        class VboBlock;
        
        class SizeGuideFigure : public Figure {
        protected:
            FontDescriptor m_fontDescriptor;
            FontManager& m_fontManager;
            std::vector<StringRendererPtr> m_strings;
            bool m_stringsValid;
            
            BBox m_bounds;
            Vec4f m_color;
            Vec4f m_hiddenColor;
            float m_offset;
            float m_cutoffDistance;
        public:
            SizeGuideFigure (FontManager& fontManager, const FontDescriptor& fontDescriptor);
            ~SizeGuideFigure();
            
            void setBounds(const BBox& bounds);
            void setColor(const Vec4f& color);
            void setHiddenColor(const Vec4f& hiddenColor);
            void render(RenderContext& context, Vbo& vbo);
        };
    }
}

#endif
