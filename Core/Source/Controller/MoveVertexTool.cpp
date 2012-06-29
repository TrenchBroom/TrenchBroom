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

#include "MoveVertexTool.h"

#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/BrushVertexFigure.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void MoveVertexTool::selectionChanged(const Model::SelectionEventData& event) {
            assert(m_figure != NULL);
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            m_figure->setBrushes(selection.brushes());
        }

        void MoveVertexTool::cleanup() {
            if (m_figure != NULL) {
                removeFigure(*m_figure);
                delete m_figure;
                m_figure = NULL;
            }
            
            if (m_listenerActive) {
                Model::Map& map = m_editor.map();
                Model::Selection& selection = map.selection();
                selection.selectionAdded -= new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
                selection.selectionRemoved -= new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
                m_listenerActive = false;
            }
        }

        MoveVertexTool::MoveVertexTool(Controller::Editor& editor) : DragTool(editor), m_brush(NULL), m_index(-1), m_figure(NULL), m_listenerActive(false) {
        }
        
        MoveVertexTool::~MoveVertexTool() {
            cleanup();
        }
        
        void MoveVertexTool::activated(ToolEvent& event) {
            cleanup();
            
            m_figure = new Renderer::BrushVertexFigure();
            addFigure(*m_figure);
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            m_figure->setBrushes(selection.brushes());

            selection.selectionAdded += new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
            selection.selectionRemoved += new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
            m_listenerActive = true;
        }
        
        void MoveVertexTool::deactivated(ToolEvent& event) {
            cleanup();
        }

        bool MoveVertexTool::doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint) {
            Model::Hit* hit = event.hits->first(Model::TB_HT_VERTEX_HANDLE, false);
            if (hit == NULL)
                return false;
            
            m_brush = &hit->brush();
            m_index = hit->index;
            initialPoint = hit->hitPoint;
            
            m_editor.map().undoManager().begin("Move Vertex");
            return true;
        }
        
        bool MoveVertexTool::doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(m_brush != NULL);
            assert(m_index != -1);
            
            Model::Vertex* vertex = m_brush->geometry->vertices[m_index];
            
            Grid& grid = m_editor.grid();
            Vec3f delta = grid.moveDelta(vertex->position, m_editor.map().worldBounds(), referencePoint, curMousePoint);
            if (delta.null())
                return true;
            
            Model::MoveResult result = m_editor.map().moveVertex(*m_brush, m_index, delta);
            m_index = result.index;
            if (result.index == -1)
                return false;
            else if (result.moved)
                referencePoint += delta;
            
            return true;
        }
        
        void MoveVertexTool::doEndLeftDrag(ToolEvent& event) {
            m_editor.map().undoManager().end();
            
            m_brush = NULL;
            m_index = -1;
        }
    }
}