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

#include "MoveFaceTool.h"

#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Renderer/Figures/HandleFigure.h"

namespace TrenchBroom {
    namespace Controller {
        int MoveFaceTool::hitType() {
            return Model::TB_HT_FACE_HANDLE;
        }
        
        std::string MoveFaceTool::undoName() {
            return "Move Face";
        }
        
        Vec3f MoveFaceTool::movePosition(const Model::Brush& brush, int index) {
            return brush.geometry->edges[index]->center();
        }
        
        const Vec4f& MoveFaceTool::handleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.faceHandleColor();
        }
        
        const Vec4f& MoveFaceTool::hiddenHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenFaceHandleColor();
        }
        
        const Vec4f& MoveFaceTool::selectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.selectedFaceHandleColor();
        }
        
        const Vec4f& MoveFaceTool::hiddenSelectedHandleColor() {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            return prefs.hiddenSelectedFaceHandleColor();
        }
        
        Model::MoveResult MoveFaceTool::performMove(Model::Brush& brush, int index, const Vec3f& delta) {
            return m_editor.map().moveFace(brush, index, delta);
        }
        
        void MoveFaceTool::updateHandleFigure() {
            if (m_handleFigure != NULL) {
                Vec3fList positions;
                
                Model::Map& map = m_editor.map();
                Model::Selection& selection = map.selection();
                const Model::BrushList& brushes = selection.brushes();
                
                for (unsigned int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    for (unsigned int j = 0; j < brush->faces.size(); j++)
                        positions.push_back(brush->faces[j]->center());
                }
                
                m_handleFigure->setPositions(positions);
            }
        }
        
        void MoveFaceTool::updateSelectedHandleFigure(const Model::Brush& brush, int index) {
            if (m_selectedHandleFigure) {
                Vec3fList positions;
                
                if (index >= 0) {
                    Model::Face* face = brush.faces[index];
                    positions.push_back(face->center());
                }
                
                m_selectedHandleFigure->setPositions(positions);
            }
        }
    }
}
