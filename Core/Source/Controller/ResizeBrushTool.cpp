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

#include "ResizeBrushTool.h"

#include "Controller/DragPlane.h"
#include "Controller/Grid.h"
#include "Controller/Options.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/ResizeBrushToolFigure.h"

namespace TrenchBroom {
    namespace Controller {

        void ResizeBrushTool::updateDragPlane(InputEvent& event) {
            m_dragPlane = DragPlane::parallel(m_referenceFace->boundary.normal, event.ray.direction);
        }

        bool ResizeBrushTool::handleMouseDown(InputEvent& event) {
            if (event.mouseButton != TB_MB_LEFT)
                return false;
            
            if (!resizeBrushModiferPressed(event))
                return false;
            
            Model::Hit* hit = event.hits->first(Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;

            Model::Face& face = hit->face();
            Model::Brush& brush = *face.brush;
            if (!face.selected && !brush.selected)
                return false;
            
            if (!m_figureCreated) {
                Renderer::ResizeBrushToolFigure* figure = new Renderer::ResizeBrushToolFigure(*this);
                addFigure(*figure);
                m_figureCreated = true;
            }
            refreshFigure(true);
            
            return false; // don't prevent the click from reaching other tools
        }
        
        bool ResizeBrushTool::handleMouseUp(InputEvent& event) {
            if (event.mouseButton != TB_MB_LEFT)
                return false;
            
            refreshFigure(false);

            return false;
        }
        
        bool ResizeBrushTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != TB_MB_LEFT || !resizeBrushModiferPressed(event))
                return false;
                
            Model::Hit* hit = event.hits->first(Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            Model::Face& face = hit->face();
            Model::Brush& brush = *face.brush;
            if (!face.selected && !brush.selected)
                return false;
            
            m_referenceFace = &face;
            initialPoint = hit->hitPoint;
            editor().map().undoManager().begin("Resize Brushes");
            return true;
        }
        
        bool ResizeBrushTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            Vec3f delta = curMousePoint - referencePoint;
            float dist = editor().grid().moveDistance(*m_referenceFace, delta);

            if (!Math::isnan(dist)) {
                Model::Selection& selection = editor().map().selection();
                Model::FaceList faces;
                if (selection.mode() == Model::TB_SM_FACES) {
                    faces = selection.faces();
                } else {
                    const Model::FaceList selectedFaces = selection.allFaces();
                    for (unsigned int i = 0; i < selectedFaces.size(); i++) {
                        Model::Face* face = selectedFaces[i];
                        if (Math::fpos(face->boundary.normal | m_referenceFace->boundary.normal))
                            faces.push_back(face);
                    }
                    
                }
                
                editor().map().resizeBrushes(faces, dist, editor().options().lockTextures());
                referencePoint += delta;

                refreshFigure(true);
            }
            
            return true;
        }
        
        void ResizeBrushTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            editor().map().undoManager().end();
            refreshFigure(false);
            m_referenceFace = NULL;
        }
        
        ResizeBrushTool::ResizeBrushTool(Editor& editor) : DragTool(editor), m_figureCreated(false) {}
        
        ResizeBrushTool::~ResizeBrushTool() {}

        bool ResizeBrushTool::resizeBrushModiferPressed(InputEvent& event) {
            return event.modifierKeys == Model::Preferences::sharedPreferences->resizeToolKey();
        }
    }
}