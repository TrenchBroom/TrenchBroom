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
#include "Model/Map/Brush.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/HandleFigure.h"
#include "Renderer/Figures/PointGuideFigure.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        size_t VertexTool::index(Model::Hit& hit) {
            return hit.index;
        }

        void VertexTool::brushesDidChange(const Model::BrushList& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++)
                if (brushes[i]->selected) {
                    if (m_selected || m_state == TS_DRAG)
                        updateSelectedHandleFigures(*m_selectedHandleFigure, *m_guideFigure, *m_brush, m_index);
                    else if (m_active)
                        updateHandleFigure(*m_handleFigure);
                    break;
                }
        }
        
        void VertexTool::selectionChanged(const Model::SelectionEventData& event) {
            if (m_active)
                updateHandleFigure(*m_handleFigure);
            if (m_selected || m_state == TS_DRAG)
                updateSelectedHandleFigures(*m_selectedHandleFigure, *m_guideFigure, *m_brush, m_index);
        }

        void VertexTool::createHandleFigure() {
            deleteHandleFigure();
            
            m_handleFigure = new Renderer::HandleFigure();
            m_handleFigure->setColor(handleColor());
            m_handleFigure->setHiddenColor(hiddenHandleColor());
            updateHandleFigure(*m_handleFigure);
            addFigure(*m_handleFigure);
        }
        
        void VertexTool::deleteHandleFigure() {
            if (m_handleFigure != NULL) {
                removeFigure(*m_handleFigure);
                delete m_handleFigure;
                m_handleFigure = NULL;
            }
        }
        
        void VertexTool::createSelectedHandleFigures() {
            assert(m_brush != NULL);
            assert(m_index >= 0);
            
            deleteSelectedHandleFigures();

            m_selectedHandleFigure = new Renderer::HandleFigure();
            m_selectedHandleFigure->setColor(selectedHandleColor());
            m_selectedHandleFigure->setHiddenColor(hiddenSelectedHandleColor());

            m_guideFigure = new Renderer::PointGuideFigure();
            m_guideFigure->setColor(selectedHandleColor());
            m_guideFigure->setHiddenColor(hiddenSelectedHandleColor());

            updateSelectedHandleFigures(*m_selectedHandleFigure, *m_guideFigure, *m_brush, m_index);
            addFigure(*m_selectedHandleFigure);
            addFigure(*m_guideFigure);
        }
        
        void VertexTool::deleteSelectedHandleFigures() {
            if (m_selectedHandleFigure != NULL) {
                removeFigure(*m_selectedHandleFigure);
                delete m_selectedHandleFigure;
                m_selectedHandleFigure = NULL;
            }
            
            if (m_guideFigure != NULL) {
                removeFigure(*m_guideFigure);
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }
        
        VertexTool::VertexTool(Controller::Editor& editor) : DragTool(editor), m_selected(false), m_brush(NULL), m_index(-1), m_handleFigure(NULL), m_selectedHandleFigure(NULL), m_guideFigure(NULL) {
        }

        bool VertexTool::handleActivated(InputEvent& event) {
            assert(m_state == TS_DEFAULT);
            
            createHandleFigure();
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            map.brushesDidChange        += new Model::Map::BrushEvent::Listener<VertexTool>(this, &VertexTool::brushesDidChange);
            selection.selectionAdded    += new Model::Selection::SelectionEvent::Listener<VertexTool>(this, &VertexTool::selectionChanged);
            selection.selectionRemoved  += new Model::Selection::SelectionEvent::Listener<VertexTool>(this, &VertexTool::selectionChanged);
            
            return true;
        }
        
        bool VertexTool::handleDeactivated(InputEvent& event) {
            assert(m_active);

            deleteHandleFigure();
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            map.brushesDidChange        -= new Model::Map::BrushEvent::Listener<VertexTool>(this, &VertexTool::brushesDidChange);
            selection.selectionAdded    -= new Model::Selection::SelectionEvent::Listener<VertexTool>(this, &VertexTool::selectionChanged);
            selection.selectionRemoved  -= new Model::Selection::SelectionEvent::Listener<VertexTool>(this, &VertexTool::selectionChanged);
            
            return true;
        }
        
        bool VertexTool::handleMouseDown(InputEvent& event) {
            assert(m_active);

            if (event.mouseButton != MB_LEFT)
                return false;
            
            Model::Hit* hit = event.hits->first(hitType(), true);
            if (hit != NULL) {
                m_brush = &hit->brush();
                m_index = index(*hit);
                deleteHandleFigure();
                createSelectedHandleFigures();
                m_selected = true;
                return true;
            }
            
            return false;
        }
        
        bool VertexTool::handleMouseUp(InputEvent& event) {
            assert(m_active);

            if (event.mouseButton != MB_LEFT)
                return false;
            
            if (m_selected) {
                deleteSelectedHandleFigures();
                createHandleFigure();
                m_brush = NULL;
                m_index = 0;
                m_selected = false;
                return true;
            }
            
            return false;
        }
        
        bool VertexTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != MB_LEFT)
                return false;
            
            Model::Hit* hit = event.hits->first(hitType(), true);
            if (hit == NULL)
                return true;
            
            assert(m_selected);

            m_brush = &hit->brush();
            m_index = index(*hit);
            initialPoint = hit->hitPoint;
            
            deleteHandleFigure();
           
            m_editor.map().undoManager().begin(undoName());
            return true;
        }
        
        bool VertexTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == MB_LEFT);
            
            if (m_state == TS_DRAG) {
                assert(m_brush != NULL);
                
                Grid& grid = m_editor.grid();
                Vec3f position = movePosition(*m_brush, m_index);
                Vec3f delta = grid.moveDelta(position, m_editor.map().worldBounds(), curMousePoint - referencePoint);
                
                if (delta.null())
                    return true;
                
                Model::MoveResult result = performMove(*m_brush, m_index, delta);
                m_index = result.index;
                if (result.deleted)
                    return false;
                else if (result.moved)
                    referencePoint += delta;
                
                updateSelectedHandleFigures(*m_selectedHandleFigure, *m_guideFigure, *m_brush, m_index);
            }
            
            return true;
        }
        
        void VertexTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == MB_LEFT);
            
            if (m_state == TS_DRAG) {
                m_editor.map().undoManager().end();
                
                deleteSelectedHandleFigures();
                createHandleFigure();
                m_brush = NULL;
                m_index = 0;
            }
            
            m_selected = false;
        }
    }
}
