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

#include "VertexTool.h"

#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/VertexToolFigure.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        size_t VertexTool::index(Model::Hit& hit) {
            return hit.index;
        }

        void VertexTool::brushesDidChange(const Model::BrushList& brushes) {
            refreshFigure(true);
        }
        
        void VertexTool::mapCleared(Model::Map& map) {
            refreshFigure(true);
        }
        
        void VertexTool::selectionChanged(const Model::SelectionEventData& event) {
            refreshFigure(true);
        }

        VertexTool::VertexTool(Controller::Editor& editor) : DragTool(editor), m_selected(false), m_brush(NULL), m_index(-1), m_figureCreated(false) {
        }

        bool VertexTool::handleActivated(InputEvent& event) {
            assert(state() == TB_TS_DEFAULT);
            
            Model::Map& map = editor().map();
            Model::Selection& selection = map.selection();
            
            map.brushesDidChange        += new BrushListener(this, &VertexTool::brushesDidChange);
            map.mapCleared              += new MapListener(this, &VertexTool::mapCleared);
            selection.selectionAdded    += new SelectionListener(this, &VertexTool::selectionChanged);
            selection.selectionRemoved  += new SelectionListener(this, &VertexTool::selectionChanged);
            
            if (!m_figureCreated) {
                Renderer::VertexToolFigure* figure = new Renderer::VertexToolFigure(*this);
                addFigure(*figure);
                m_figureCreated = true;
            }

            refreshFigure(true);

            return true;
        }
        
        bool VertexTool::handleDeactivated(InputEvent& event) {
            assert(active());

            Model::Map& map = editor().map();
            Model::Selection& selection = map.selection();
            
            map.brushesDidChange        -= new BrushListener(this, &VertexTool::brushesDidChange);
            map.mapCleared              -= new MapListener(this, &VertexTool::mapCleared);
            selection.selectionAdded    -= new SelectionListener(this, &VertexTool::selectionChanged);
            selection.selectionRemoved  -= new SelectionListener(this, &VertexTool::selectionChanged);
            
            refreshFigure(true);
            
            return true;
        }
        
        bool VertexTool::handleMouseDown(InputEvent& event) {
            assert(active());

            if (event.mouseButton != TB_MB_LEFT || !noModifierPressed(event))
                return false;
            
            Model::Hit* hit = event.hits->first(hitType(), true);
            if (hit != NULL) {
                m_brush = &hit->brush();
                m_index = index(*hit);
                m_selected = true;
                refreshFigure(true);
                return true;
            }
            
            return false;
        }
        
        bool VertexTool::handleMouseUp(InputEvent& event) {
            assert(active());

            if (event.mouseButton != TB_MB_LEFT)
                return false;
            
            if (selected()) {
                m_brush = NULL;
                m_index = 0;
                m_selected = false;
                refreshFigure(false);
                return true;
            }
            
            return false;
        }
        
        bool VertexTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != TB_MB_LEFT || !noModifierPressed(event))
                return false;
            
            Model::Hit* hit = event.hits->first(hitType(), true);
            if (hit == NULL)
                return false;
            
            assert(selected());

            m_brush = &hit->brush();
            m_index = index(*hit);
            initialPoint = hit->hitPoint;
            
            editor().map().undoManager().begin(undoName());
            return true;
        }
        
        bool VertexTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            if (state() == TB_TS_DRAG) {
                assert(m_brush != NULL);
                
                Vec3f position = movePosition(*m_brush, m_index);
                Vec3f delta = moveDelta(position, curMousePoint - referencePoint);
                
                if (delta.null())
                    return true;
                
                Model::MoveResult result = performMove(*m_brush, m_index, delta);
                m_index = result.index;
                if (result.deleted)
                    return false;
                else if (result.moved)
                    referencePoint += delta;

                refreshFigure(result.moved);
            }
            
            return true;
        }
        
        void VertexTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            if (state() == TB_TS_DRAG) {
                editor().map().undoManager().end();
                m_brush = NULL;
                m_index = 0;
            }
            
            m_selected = false;
            refreshFigure(true);
        }
    }
}
