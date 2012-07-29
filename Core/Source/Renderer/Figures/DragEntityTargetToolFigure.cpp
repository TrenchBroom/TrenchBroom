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

#include "DragEntityTargetToolFigure.h"

#include "Controller/DragEntityTargetTool.h"
#include "Renderer/Figures/EntityFigure.h"
#include "Renderer/Figures/BoundsGuideFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        DragEntityTargetToolFigure::DragEntityTargetToolFigure(Controller::DragEntityTargetTool& dragEntityTargetTool) : m_dragEntityTargetTool(dragEntityTargetTool), m_entityFigure(NULL), m_guideFigure(NULL) {}

        DragEntityTargetToolFigure::~DragEntityTargetToolFigure() {
            if (m_entityFigure != NULL) {
                delete m_entityFigure;
                m_entityFigure = NULL;
            }
            
            if (m_guideFigure != NULL) {
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }

        void DragEntityTargetToolFigure::render(RenderContext& context, Vbo& vbo) {
            if (!m_dragEntityTargetTool.active())
                return;
            
            if (m_entityFigure == NULL) {
                m_entityFigure = new EntityFigure(m_dragEntityTargetTool.editor());
                m_entityFigure->setRenderBounds(false);
            }
            
            if (m_guideFigure == NULL) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_guideFigure = new BoundsGuideFigure();
                m_guideFigure->setColor(prefs.selectionGuideColor());
                m_guideFigure->setHiddenColor(prefs.hiddenSelectionGuideColor());
            }
            
            if (!m_dragEntityTargetTool.checkFigureDataValid()) {
                m_entityFigure->setEntityDefinition(m_dragEntityTargetTool.entityDefinition());
                m_entityFigure->setPosition(m_dragEntityTargetTool.position());
                m_guideFigure->setBounds(m_dragEntityTargetTool.bounds());
            }
            
            m_entityFigure->render(context, vbo);
            m_guideFigure->render(context, vbo);
        }
    }
}
