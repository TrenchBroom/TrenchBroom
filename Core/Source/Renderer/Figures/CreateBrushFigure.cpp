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

#include "CreateBrushFigure.h"

#include "Controller/CreateBrushTool.h"
#include "Model/Map/Brush.h"
#include "Model/Preferences.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/Figures/BrushFigure.h"
#include "Renderer/Figures/SizeGuideFigure.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Renderer {
        CreateBrushFigure::CreateBrushFigure(Controller::CreateBrushTool& createBrushTool) : m_createBrushTool(createBrushTool), m_brushFigure(NULL), m_sizeGuideFigure(NULL) {
        }
        
        CreateBrushFigure::~CreateBrushFigure() {
            if (m_brushFigure != NULL) {
                delete m_brushFigure;
                m_brushFigure = NULL;
            }
            
            if (m_sizeGuideFigure != NULL) {
                delete m_sizeGuideFigure;
                m_sizeGuideFigure = NULL;
            }
        }
        
        void CreateBrushFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_createBrushTool.state() != Controller::Tool::TB_TS_DRAG)
                return;
            
			log(TB_LL_DEBUG, "render create brush figure\n");

            if (m_brushFigure == NULL) {
				log(TB_LL_DEBUG, "create brush figure\n");
                m_brushFigure = new BrushFigure();
            }
            
            if (m_sizeGuideFigure == NULL) {
				log(TB_LL_DEBUG, "create size guide figure\n");
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                FontManager& fontManager = m_createBrushTool.editor().renderer()->fontManager();
                m_sizeGuideFigure = new SizeGuideFigure(fontManager, FontDescriptor(prefs.rendererFontName(), prefs.rendererFontSize()));
                m_sizeGuideFigure->setColor(prefs.selectionGuideColor());
            }
            
            if (!m_createBrushTool.checkFigureDataValid()) {
				log(TB_LL_DEBUG, "Update brush figure\n");
                
				Model::BrushList brushes;
                brushes.push_back(m_createBrushTool.brush());
                
				log(TB_LL_DEBUG, "Update size guide figure\n");
                m_brushFigure->setBrushes(brushes);
                m_sizeGuideFigure->setBounds(m_createBrushTool.bounds());
            }
            
			log(TB_LL_DEBUG, "render brush figure\n");
            m_brushFigure->render(context, vbo);

			log(TB_LL_DEBUG, "render size guide figure\n");
            m_sizeGuideFigure->render(context, vbo);
        }
    }
}