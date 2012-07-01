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
#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Renderer/Figures/HandleFigure.h"

namespace TrenchBroom {
    namespace Controller {
        Model::EHitType MoveVertexTool::hitType() {
            return Model::TB_HT_VERTEX_HANDLE;
        }
        
        std::string MoveVertexTool::undoName() {
            return "Move Vertex";
        }
        
        Vec3f MoveVertexTool::movePosition(const Model::Brush& brush, int index) {
            int vertexCount = static_cast<int>(brush.geometry->vertices.size());
            int edgeCount = static_cast<int>(brush.geometry->edges.size());
            
            if (index < vertexCount) {
                Model::Vertex* vertex = brush.geometry->vertices[index];
                return vertex->position;
            } else if (index < vertexCount + edgeCount) {
                Model::Edge* edge = brush.geometry->edges[index - vertexCount];
                return edge->center();
            } else {
                Model::Face* face = brush.faces[index - vertexCount - edgeCount];
                return face->center();
            }
        }
        
        const Vec4f& MoveVertexTool::handleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.vertexHandleColor();
        }
        
        const Vec4f& MoveVertexTool::hiddenHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenVertexHandleColor();
        }
        
        const Vec4f& MoveVertexTool::selectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.selectedVertexHandleColor();
        }
        
        const Vec4f& MoveVertexTool::hiddenSelectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenSelectedVertexHandleColor();
        }

        Model::MoveResult MoveVertexTool::performMove(Model::Brush& brush, int index, const Vec3f& delta) {
            return m_editor.map().moveVertex(brush, index, delta);
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
    }
}