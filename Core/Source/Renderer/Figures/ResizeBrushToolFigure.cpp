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

#include "ResizeBrushToolFigure.h"

#include "Controller/Editor.h"
#include "Controller/ResizeBrushTool.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"
#include "Renderer/Figures/BoundsGuideFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        ResizeBrushToolFigure::ResizeBrushToolFigure(Controller::ResizeBrushTool& resizeBrushTool) : m_resizeBrushTool(resizeBrushTool), m_guideFigure(NULL) {}
        
        ResizeBrushToolFigure::~ResizeBrushToolFigure() {
            if (m_guideFigure != NULL) {
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }
        
        void ResizeBrushToolFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_resizeBrushTool.state() != Controller::Tool::TS_MOUSE_DOWN && m_resizeBrushTool.state() != Controller::Tool::TS_DRAG)
                return;
            
            if (m_guideFigure == NULL) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_guideFigure = new BoundsGuideFigure();
                m_guideFigure->setColor(prefs.selectionGuideColor());
                m_guideFigure->setHiddenColor(prefs.hiddenSelectionGuideColor());
            }
            
            if (!m_resizeBrushTool.checkFigureDataValid()) {
                BBox bounds = m_resizeBrushTool.editor().map().selection().bounds();
                m_guideFigure->setBounds(bounds);
            }
            
            m_guideFigure->render(context, vbo);
        }
    }
}