#/*
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

#include "CreateBrushTool.h"

#include "Controller/Camera.h"
#include "Controller/Grid.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Renderer/Figures/CreateBrushFigure.h"
#include "Renderer/MapRenderer.h"

namespace TrenchBroom {
    namespace Controller {
        CreateBrushTool::CreateBrushTool(Editor& editor) : DragTool(editor), m_brush(NULL), m_figureCreated(false) {
        }
        
        CreateBrushTool::~CreateBrushTool() {
            if (m_brush != NULL) {
                delete m_brush;
                m_brush = NULL;
            }
        }
        
        void CreateBrushTool::updateBrush() {
            if (m_brush != NULL)
                delete m_brush;
            
            Model::Map& map = editor().map();
            Model::Selection& selection = map.selection();
            Model::Assets::Texture* texture = selection.texture();
            m_brush = new Model::Brush(map.worldBounds(), m_bounds, texture);
        }
        
        bool CreateBrushTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != TB_MB_RIGHT)
                return false;
            
            if (!m_figureCreated) {
                Renderer::CreateBrushFigure* figure = new Renderer::CreateBrushFigure(*this);
                addFigure(*figure);
                m_figureCreated = true;
            }
            
            editor().map().selection().deselectAll();
            
            Model::Hit* hit = event.pickResults->first(Model::TB_HT_FACE, true);
            if (hit != NULL)
                initialPoint = hit->hitPoint.correct();
            else
                initialPoint = editor().camera().defaultPoint(event.ray.direction).correct();
            
            m_initialBounds.min = initialPoint;
            m_initialBounds.max = initialPoint;
            
            m_initialBounds.min = editor().grid().snapDown(m_initialBounds.min);
            m_initialBounds.max = editor().grid().snapUp(m_initialBounds.max);
            
            if (m_initialBounds.min.x == m_initialBounds.max.x) {
                if (event.ray.direction.x > 0) m_initialBounds.min.x -= editor().grid().actualSize();
                else m_initialBounds.max.x += editor().grid().actualSize();
            }
            
            if (m_initialBounds.min.y == m_initialBounds.max.y) {
                if (event.ray.direction.y > 0) m_initialBounds.min.y -= editor().grid().actualSize();
                else m_initialBounds.max.y += editor().grid().actualSize();
            }
            
            if (m_initialBounds.min.z == m_initialBounds.max.z) {
                if (event.ray.direction.z > 0) m_initialBounds.min.z -= editor().grid().actualSize();
                else m_initialBounds.max.z += editor().grid().actualSize();
            }

            m_bounds = m_initialBounds;
            updateBrush();
            refreshFigure(true);
            
            return true;
        }
        
        bool CreateBrushTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == TB_MB_RIGHT);
            
            BBox newBounds = m_initialBounds + curMousePoint.correct();
            newBounds.min = editor().grid().snapDown(newBounds.min);
            newBounds.max = editor().grid().snapUp(newBounds.max);
            if (m_bounds == newBounds) return true;
            
            m_bounds = newBounds;
            updateBrush();
            refreshFigure(true);
            
            return true;
        }
        
        void CreateBrushTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == TB_MB_RIGHT);

            editor().map().createBrush(*editor().map().worldspawn(true), *m_brush);

            if (m_brush != NULL) {
                delete m_brush;
                m_brush = NULL;
            }

            refreshFigure(true);
        }
    }
}
