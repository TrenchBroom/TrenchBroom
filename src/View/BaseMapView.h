/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__BaseMapView__
#define __TrenchBroom__BaseMapView__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Hit.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "View/MovementRestriction.h"
#include "View/ToolView.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Renderer {
        class Camera;
        class RenderContext;
    }
    
    namespace View {
        class AnimationManager;
        class Tool;
        
        class BaseMapView : public ToolView {
        protected:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            MovementRestriction m_movementRestriction;
        protected:
            BaseMapView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext = NULL);
        public:
            virtual ~BaseMapView();
            void OnKey(wxKeyEvent& event);

            void toggleMovementRestriction();
        private:
            void bindObservers();
            void unbindObservers();

            void documentWasNewedOrLoaded();
            void objectWasAddedOrDidChange(Model::Object* object);
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            void commandDoneOrUndone(Controller::Command::Ptr command);
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);
            void cameraDidChange(const Renderer::Camera* camera);

            void bindEvents();
        private:
            void doUpdateViewport(int x, int y, int width, int height);
            Ray3d doGetPickRay(int x, int y) const;
            Hits doGetHits(const Ray3d& pickRay) const;
        };
    }
}

#endif /* defined(__TrenchBroom__BaseMapView__) */
