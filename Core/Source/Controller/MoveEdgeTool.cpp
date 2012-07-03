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

namespace TrenchBroom {
    namespace Controller {
        int MoveEdgeTool::hitType() {
            return Model::TB_HT_EDGE_HANDLE;
        }
        
        std::string MoveEdgeTool::undoName() {
            return "Move Edge";
        }
        
        Vec3f MoveEdgeTool::movePosition(const Model::Brush& brush, int index) {
            return brush.geometry->edges[index]->center();
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
        
        Model::MoveResult MoveEdgeTool::performMove(Model::Brush& brush, int index, const Vec3f& delta) {
            return m_editor.map().moveEdge(brush, index, delta);
        }
        
        void MoveEdgeTool::updateHandleFigure() {
            if (m_handleFigure != NULL) {
                Vec3fList positions;
                
                Model::Map& map = m_editor.map();
                Model::Selection& selection = map.selection();
                const Model::BrushList& brushes = selection.brushes();
                
                for (unsigned int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    const Model::EdgeList& edges = brush->geometry->edges;
                    for (unsigned int j = 0; j < edges.size(); j++)
                        positions.push_back(edges[j]->center());
                }
                
                m_handleFigure->setPositions(positions);
            }
        }
        
        void MoveEdgeTool::updateSelectedHandleFigure(const Model::Brush& brush, int index) {
            if (m_selectedHandleFigure) {
                Vec3fList positions;
                
                if (index >= 0) {
                    Model::Edge* edge = brush.geometry->edges[index];
                    positions.push_back(edge->center());
                }
                
                m_selectedHandleFigure->setPositions(positions);
            }
        }
    }
}
