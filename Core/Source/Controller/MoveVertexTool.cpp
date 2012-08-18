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
#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Renderer/Figures/HandleFigure.h"
#include "Renderer/Figures/PointGuideFigure.h"

namespace TrenchBroom {
    namespace Controller {
        int MoveVertexTool::hitType() {
            return Model::TB_HT_VERTEX_HANDLE | Model::TB_HT_EDGE_HANDLE | Model::TB_HT_FACE_HANDLE;
        }
        
        size_t MoveVertexTool::index(Model::Hit& hit) {
            if (hit.type == Model::TB_HT_VERTEX_HANDLE)
                return hit.index;

            Model::Brush& brush = hit.brush();
            if (hit.type == Model::TB_HT_EDGE_HANDLE)
                return brush.geometry->vertices.size() + hit.index;
            return brush.geometry->vertices.size() + brush.geometry->edges.size() + hit.index;
        }
        
        std::string MoveVertexTool::undoName() {
            return "Move Vertex";
        }
        
        Vec3f MoveVertexTool::movePosition(const Model::Brush& brush, size_t index) {
            size_t vertexCount = brush.geometry->vertices.size();
            size_t edgeCount = brush.geometry->edges.size();
            
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
        
        Vec3f MoveVertexTool::moveDelta(const Vec3f& position, const Vec3f& delta) {
            return editor().grid().moveDelta(position, editor().map().worldBounds(), delta);
        }

        Model::MoveResult MoveVertexTool::performMove(Model::Brush& brush, size_t index, const Vec3f& delta) {
            return editor().map().moveVertex(brush, index, delta);
        }

        void MoveVertexTool::updateHits(InputEvent& event) {
            if (!active())
                return;
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            
            Model::Selection& selection = editor().map().selection();
            const Model::BrushList& brushes = selection.selectedBrushes();
            for (unsigned int i = 0; i < brushes.size(); i++)
                brushes[i]->pickVertexHandles(event.ray, prefs.vertexHandleSize(), *event.pickResults);
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
        
        const Vec3fList MoveVertexTool::handlePositions() {
            Vec3fList positions;
            
            Model::Map& map = editor().map();
            Model::Selection& selection = map.selection();
            const Model::BrushList& brushes = selection.selectedBrushes();
            
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
            
            return positions;
        }
        
        const Vec3fList MoveVertexTool::selectedHandlePositions() {
            Vec3fList positions;
            positions.push_back(draggedHandlePosition());
            return positions;
        }
        
        const Vec3f MoveVertexTool::draggedHandlePosition() {
            size_t vertexCount = brush()->geometry->vertices.size();
            size_t edgeCount = brush()->geometry->edges.size();
            size_t faceCount = brush()->faces.size();
            size_t index = VertexTool::index();
            
            if (index < vertexCount) {
                return brush()->geometry->vertices[index]->position;
            } else if (index < vertexCount + edgeCount) {
                Model::Edge* edge = brush()->geometry->edges[index - vertexCount];
                return edge->center();
            } else if (index < vertexCount + edgeCount + faceCount) {
                Model::Face* face = brush()->faces[index - vertexCount - edgeCount];
                return face->center();
            }
            
            return Vec3f::Null;
        }
    }
}