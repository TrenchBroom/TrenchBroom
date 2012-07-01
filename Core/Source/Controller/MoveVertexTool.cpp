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
#include "Model/Preferences.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/HandleFigure.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void MoveVertexTool::brushesDidChange(const Model::BrushList& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++)
                if (brushes[i]->selected) {
                    updateHandleFigure();
                    updateSelectedHandleFigure();
                    break;
                }
        }

        void MoveVertexTool::selectionChanged(const Model::SelectionEventData& event) {
            assert(m_handleFigure != NULL);
            updateHandleFigure();
            updateSelectedHandleFigure();
        }

        void MoveVertexTool::createHandleFigure() {
            deleteHandleFigure();
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            m_handleFigure = new Renderer::HandleFigure();
            m_handleFigure->setColor(prefs.vertexHandleColor());
            m_handleFigure->setHiddenColor(prefs.hiddenVertexHandleColor());
            updateHandleFigure();
            addFigure(*m_handleFigure);
        }
        
        void MoveVertexTool::updateHandleFigure() {
            if (m_handleFigure != NULL) {
                Vec3fList positions;
                
                Model::Map& map = m_editor.map();
                Model::Selection& selection = map.selection();
                const Model::BrushList& brushes = selection.brushes();
                
                for (unsigned int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    const Model::VertexList& vertices = brush->geometry->vertices;
                    for (unsigned int j = 0; j < vertices.size(); j++)
                        positions.push_back(vertices[j]->position);
                    
                    const Model::EdgeList& edges = brush->geometry->edges;
                    for (unsigned int j = 0; j < edges.size(); j++)
                        positions.push_back(edges[j]->center());
                    
                    for (unsigned int j = 0; j < brush->faces.size(); j++)
                        positions.push_back(brush->faces[j]->center());
                }
                
                m_handleFigure->setPositions(positions);
            }
        }

        void MoveVertexTool::deleteHandleFigure() {
            if (m_handleFigure != NULL) {
                removeFigure(*m_handleFigure);
                delete m_handleFigure;
                m_handleFigure = NULL;
            }
        }
        
        void MoveVertexTool::createSelectedHandleFigure() {
            deleteSelectedHandleFigure();
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            m_selectedHandleFigure = new Renderer::HandleFigure();
            m_selectedHandleFigure->setColor(prefs.selectedVertexHandleColor());
            m_selectedHandleFigure->setHiddenColor(prefs.hiddenSelectedVertexHandleColor());
            updateSelectedHandleFigure();
            addFigure(*m_selectedHandleFigure);
        }
        
        void MoveVertexTool::updateSelectedHandleFigure() {
            if (m_selectedHandleFigure) {
                Vec3fList positions;
                
                if (m_index >= 0) {
                    int vertexCount = static_cast<int>(m_brush->geometry->vertices.size());
                    int edgeCount = static_cast<int>(m_brush->geometry->edges.size());
                    int faceCount = static_cast<int>(m_brush->faces.size());
                    
                    if (m_index < vertexCount) {
                        positions.push_back(m_brush->geometry->vertices[m_index]->position);
                    } else if (m_index < vertexCount + edgeCount) {
                        Model::Edge* edge = m_brush->geometry->edges[m_index - vertexCount];
                        positions.push_back(edge->center());
                    } else if (m_index < vertexCount + edgeCount + faceCount) {
                        Model::Face* face = m_brush->faces[m_index - vertexCount - edgeCount];
                        positions.push_back(face->center());
                    }
                }
                
                m_selectedHandleFigure->setPositions(positions);
            }
        }

        void MoveVertexTool::deleteSelectedHandleFigure() {
            if (m_selectedHandleFigure != NULL) {
                removeFigure(*m_selectedHandleFigure);
                delete m_selectedHandleFigure;
                m_selectedHandleFigure = NULL;
            }
        }
        
        void MoveVertexTool::cleanup() {
            deleteHandleFigure();
            deleteSelectedHandleFigure();
            
            if (m_listenerActive) {
                Model::Map& map = m_editor.map();
                map.brushesDidChange -= new Model::Map::BrushEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::brushesDidChange);
                
                Model::Selection& selection = map.selection();
                selection.selectionAdded -= new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
                selection.selectionRemoved -= new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
                m_listenerActive = false;
            }
        }

        MoveVertexTool::MoveVertexTool(Controller::Editor& editor) : DragTool(editor), m_brush(NULL), m_index(-1), m_handleFigure(NULL), m_selectedHandleFigure(NULL), m_listenerActive(false) {
        }
        
        MoveVertexTool::~MoveVertexTool() {
            cleanup();
        }
        
        void MoveVertexTool::activated(ToolEvent& event) {
            cleanup();
            
            createHandleFigure();
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            map.brushesDidChange        += new Model::Map::BrushEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::brushesDidChange);
            selection.selectionAdded    += new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
            selection.selectionRemoved  += new Model::Selection::SelectionEvent::Listener<MoveVertexTool>(this, &MoveVertexTool::selectionChanged);
            m_listenerActive = true;
        }
        
        void MoveVertexTool::deactivated(ToolEvent& event) {
            cleanup();
        }

        bool MoveVertexTool::leftMouseDown(ToolEvent& event) {
            Model::Hit* hit = event.hits->first(Model::TB_HT_VERTEX_HANDLE, true);
            if (hit != NULL) {
                m_brush = &hit->brush();
                m_index = hit->index;
                deleteHandleFigure();
                createSelectedHandleFigure();
            }
            
            return false;
        }
        
        bool MoveVertexTool::leftMouseUp(ToolEvent& event) {
            deleteSelectedHandleFigure();
            createHandleFigure();
            return false;
        }

        
        bool MoveVertexTool::doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint) {
            Model::Hit* hit = event.hits->first(Model::TB_HT_VERTEX_HANDLE, true);
            if (hit == NULL)
                return false;
            
            m_brush = &hit->brush();
            m_index = hit->index;
            initialPoint = hit->hitPoint;

            deleteHandleFigure();
            createSelectedHandleFigure();
            
            m_editor.map().undoManager().begin("Move Vertex");
            return true;
        }
        
        bool MoveVertexTool::doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(m_brush != NULL);
            assert(m_index != -1);
            
            int vertexCount = static_cast<int>(m_brush->geometry->vertices.size());
            int edgeCount = static_cast<int>(m_brush->geometry->edges.size());

            Grid& grid = m_editor.grid();
            Vec3f delta;
            
            if (m_index < vertexCount) {
                Model::Vertex* vertex = m_brush->geometry->vertices[m_index];
                delta = grid.moveDelta(vertex->position, m_editor.map().worldBounds(), referencePoint, curMousePoint);
            } else if (m_index < vertexCount + edgeCount) {
                Model::Edge* edge = m_brush->geometry->edges[m_index - vertexCount];
                delta = grid.moveDelta(edge->center(), m_editor.map().worldBounds(), referencePoint, curMousePoint);
            } else {
                Model::Face* face = m_brush->faces[m_index - vertexCount - edgeCount];
                delta = grid.moveDelta(face->center(), m_editor.map().worldBounds(), referencePoint, curMousePoint);
            }
            
            if (delta.null())
                return true;
            
            Model::MoveResult result = m_editor.map().moveVertex(*m_brush, m_index, delta);
            m_index = result.index;
            if (result.index == -1)
                return false;
            else if (result.moved)
                referencePoint += delta;

            updateSelectedHandleFigure();
            
            return true;
        }
        
        void MoveVertexTool::doEndLeftDrag(ToolEvent& event) {
            m_editor.map().undoManager().end();
            
            m_brush = NULL;
            m_index = -1;

            deleteSelectedHandleFigure();
            createHandleFigure();
        }
    }
}