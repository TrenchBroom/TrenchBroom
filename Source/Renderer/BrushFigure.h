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

#ifndef __TrenchBroom__BrushFigure__
#define __TrenchBroom__BrushFigure__

#include "Model/BrushTypes.h"
#include "Renderer/Figure.h"
#include "Renderer/RenderTypes.h"
#include "Utility/Color.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    
    namespace Renderer {
        class TextureRendererManager;
        
        class BrushFigure : public Figure {
        public:
            typedef enum {
                EMDefault,
                EMOverride,
                EMRenderOccluded,
            } EdgeMode;
        private:
            TextureRendererManager& m_textureRendererManager;
            Model::BrushList m_brushes;
            FaceRendererPtr m_faceRenderer;
            EdgeRendererPtr m_edgeRenderer;
            Color m_faceColor;
            bool m_applyTinting;
            Color m_faceTintColor;
            Color m_edgeColor;
            Color m_occludedEdgeColor;
            EdgeMode m_edgeMode;
            bool m_grayScale;
            
            bool m_faceRendererValid;
            bool m_edgeRendererValid;
        public:
            BrushFigure(TextureRendererManager& textureRendererManager);

            inline void setBrushes(const Model::BrushList& brushes) {
                m_brushes = brushes;
                m_edgeRendererValid = false;
                m_faceRendererValid = false;
            }
            
            inline void setBrush(Model::Brush& brush) {
                m_brushes.clear();
                m_brushes.push_back(&brush);
                m_edgeRendererValid = false;
                m_faceRendererValid = false;
            }
            
            inline void setFaceColor(const Color& faceColor) {
                if (m_faceColor == faceColor)
                    return;
                m_faceColor = faceColor;
                m_faceRendererValid = false;
            }
            
            inline void setFaceTintColor(const Color& faceTintColor) {
                m_faceTintColor = faceTintColor;
            }
            
            inline void setApplyTinting(bool applyTinting) {
                m_applyTinting = applyTinting;
            }
            
            inline void setEdgeColor(const Color& edgeColor) {
                if (m_edgeColor == edgeColor)
                    return;
                m_edgeColor = edgeColor;
                m_edgeRendererValid = false;
            }
            
            inline void setOccludedEdgeColor(const Color& occludedEdgeColor) {
                if (m_occludedEdgeColor == occludedEdgeColor)
                    return;
                m_occludedEdgeColor = occludedEdgeColor;
                m_edgeRendererValid = false;
            }
            
            inline void setEdgeMode(EdgeMode edgeMode) {
                if (m_edgeMode == edgeMode)
                    return;
                m_edgeMode = edgeMode;
                m_edgeRendererValid = false;
            }
            
            inline void setGrayScale(bool grayScale) {
                m_grayScale = grayScale;
            }
            
            void renderFaces(Vbo& vbo, RenderContext& context);
            void renderEdges(Vbo& vbo, RenderContext& context);
            
            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__BrushFigure__) */
