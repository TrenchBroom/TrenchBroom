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

#include "MoveEdgeTool.h"

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
        int MoveEdgeTool::hitType() {
            return Model::TB_HT_EDGE_HANDLE;
        }
        
        std::string MoveEdgeTool::undoName() {
            return "Move Edge";
        }
        
        Vec3f MoveEdgeTool::movePosition(const Model::Brush& brush, size_t index) {
            return brush.geometry->edges[index]->center();
        }
        
        Model::MoveResult MoveEdgeTool::performMove(Model::Brush& brush, size_t index, const Vec3f& delta) {
            return editor().map().moveEdge(brush, index, delta);
        }
        
        const Vec4f& MoveEdgeTool::handleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.edgeHandleColor();
        }
        
        const Vec4f& MoveEdgeTool::hiddenHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenEdgeHandleColor();
        }
        
        const Vec4f& MoveEdgeTool::selectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.selectedEdgeHandleColor();
        }
        
        const Vec4f& MoveEdgeTool::hiddenSelectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenSelectedEdgeHandleColor();
        }
        
        const Vec3fList MoveEdgeTool::handlePositions() {
            Vec3fList positions;
            
            Model::Map& map = editor().map();
            Model::Selection& selection = map.selection();
            const Model::BrushList& brushes = selection.brushes();
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                const Model::EdgeList& edges = brushes[i]->geometry->edges;
                for (unsigned int j = 0; j < edges.size(); j++)
                    positions.push_back(edges[j]->center());
            }
            
            return positions;
        }
        
        const Vec3fList MoveEdgeTool::selectedHandlePositions() {
            Vec3fList positions;
            positions.push_back(draggedHandlePosition());
            return positions;
        }
        
        const Vec3f MoveEdgeTool::draggedHandlePosition() {
            size_t index = VertexTool::index();
            
            Model::Edge* edge = brush()->geometry->edges[index];
            return edge->center();
        }
    }
}
