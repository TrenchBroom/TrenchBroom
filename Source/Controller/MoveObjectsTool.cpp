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

#include "MoveObjectsTool.h"

#include "Model/Picker.h"
#include "Renderer/MoveObjectsHandleFigure.h"
#include "View/DocumentViewHolder.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectsTool::handleMouseMoved(InputEvent& event) {
            View::EditorView& view = documentViewHolder().view();
            Model::Filter& filter = view.filter();

            Model::Hit* hit = event.pickResult->first(Model::HitType::ObjectHit, false, filter);
            if (hit != NULL) {
                Vec3f position;
                if (hit->type() == Model::HitType::EntityHit)
                    position = static_cast<Model::EntityHit*>(hit)->entity().center();
                else
                    position = static_cast<Model::FaceHit*>(hit)->face().brush()->center();
                
                if (m_handleFigure == NULL) {
                    m_handleFigure = new Renderer::MoveObjectsHandleFigure();
                    addFigure(m_handleFigure);
                }
                m_handleFigure->setPosition(position);
            } else if (m_handleFigure != NULL) {
                removeFigure(m_handleFigure);
                m_handleFigure = NULL;
            }
            return false;
        }

        bool MoveObjectsTool::handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane) {
            if (event.mouseButtons != MouseButtons::MBLeft ||
                event.modifierKeys() != ModifierKeys::MKNone)
                return false;

            return true;
        }
        
        bool MoveObjectsTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            return true;
        }
        
        void MoveObjectsTool::handleEndPlaneDrag(InputEvent& event) {
        }
        
        void MoveObjectsTool::handleChangeEditState(const Model::EditStateChangeSet& changeSet) {
            
        }

        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder) :
        DragTool(documentViewHolder),
        m_handleFigure(NULL) {}
    }
}
