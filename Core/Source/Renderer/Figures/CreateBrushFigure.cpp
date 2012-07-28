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

#include "Model/Map/Brush.h"
#include "Model/Preferences.h"
#include "Renderer/Figures/BrushFigure.h"
#include "Renderer/Figures/SizeGuideFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        void CreateBrushFigure::createBrush(Model::Brush& brush) {
            m_brush = &brush;
            m_valid = false;
        }
        
        void CreateBrushFigure::modifyBrush(Model::Brush& brush) {
            m_brush = &brush;
            m_valid = false;
        }
        
        void CreateBrushFigure::finishBrush(Model::Brush& brush) {
            m_brush = NULL;
            m_valid = false;
        }
        
        CreateBrushFigure::CreateBrushFigure(Controller::CreateBrushTool& createBrushTool, FontManager& fontManager) : m_createBrushTool(createBrushTool), m_fontManager(fontManager), m_brush(NULL), m_valid(false), m_brushFigure(NULL), m_sizeGuideFigure(NULL) {
            
            m_createBrushTool.createBrush += new Listener(this, &CreateBrushFigure::createBrush);
            m_createBrushTool.modifyBrush += new Listener(this, &CreateBrushFigure::modifyBrush);
            m_createBrushTool.finishBrush += new Listener(this, &CreateBrushFigure::finishBrush);
        }
        
        CreateBrushFigure::~CreateBrushFigure() {
            m_createBrushTool.createBrush -= new Listener(this, &CreateBrushFigure::createBrush);
            m_createBrushTool.modifyBrush -= new Listener(this, &CreateBrushFigure::modifyBrush);
            m_createBrushTool.finishBrush -= new Listener(this, &CreateBrushFigure::finishBrush);

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
            if (m_brush == NULL)
                return;
            
            if (m_brushFigure == NULL) {
                m_brushFigure = new BrushFigure();
            }
            
            if (m_sizeGuideFigure == NULL) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_sizeGuideFigure = new SizeGuideFigure(m_fontManager, FontDescriptor(prefs.rendererFontName(), prefs.rendererFontSize()));
                m_sizeGuideFigure->setColor(prefs.selectionGuideColor());
            }
            
            if (!m_valid) {
                Model::BrushList brushes;
                brushes.push_back(m_brush);
                m_brushFigure->setBrushes(brushes);
                
                m_sizeGuideFigure->setBounds(m_brush->bounds());
                m_valid = true;
            }
            
            m_brushFigure->render(context, vbo);
            m_sizeGuideFigure->render(context, vbo);
        }
    }
}