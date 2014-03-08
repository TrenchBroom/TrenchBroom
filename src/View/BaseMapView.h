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
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "View/InputState.h"
#include "View/MovementRestriction.h"
#include "Renderer/RenderResources.h"
#include "View/ViewTypes.h"

#include <wx/glcanvas.h>

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderContext;
    }
    
    namespace View {
        class AnimationManager;
        class Tool;
        
        class BaseMapView : public wxGLCanvas {
        public:
            typedef std::vector<int> GLAttribs;
        protected:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            Renderer::Camera& m_camera;
            InputState m_inputState;
            MovementRestriction m_movementRestriction;
        private:
            wxGLContext* m_glContext;
            bool m_initialized;
            
            AnimationManager* m_animationManager;

            Tool* m_toolChain;
            Tool* m_dragReceiver;
            Tool* m_modalReceiver;
            Tool* m_dropReceiver;
            Tool* m_savedDropReceiver;

            wxPoint m_clickPos;
            bool m_ignoreNextDrag;
            bool m_ignoreNextClick;
            wxDateTime m_lastFrameActivation;
        public:
            BaseMapView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext = NULL);
            virtual ~BaseMapView();

            bool dragEnter(wxCoord x, wxCoord y, const String& text);
            bool dragMove(wxCoord x, wxCoord y, const String& text);
            void dragLeave();
            bool dragDrop(wxCoord x, wxCoord y, const String& text);
            
            void OnKey(wxKeyEvent& event);
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseDoubleClick(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            void OnActivateFrame(wxActivateEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);

            void toggleMovementRestriction();
            
            const wxGLContext* glContext() const;
        protected:
            void resetCamera();
        public:
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration);
            bool anyToolActive() const;
        protected:
            void addTool(Tool* tool);
            bool toolActive(const Tool* tool) const;
            void toggleTool(Tool* tool);
            void deactivateAllTools();

            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderTools(Renderer::RenderContext& renderContext);
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
        private:
            void bindEvents();
            
            void initializeGL();
            void render();
            
            void cancelCurrentDrag();
            
            ModifierKeyState modifierKeys();
            bool updateModifierKeys();
            bool clearModifierKeys();
            MouseButtonState mouseButton(wxMouseEvent& event);

            void updateHits(const int x, const int y);
            
            void showPopupMenu();
        private:
            virtual void doResetCamera() = 0;
            virtual void doInitializeGL();
            virtual void doRender() = 0;
            virtual void doShowPopupMenu();
        };
    }
}

#endif /* defined(__TrenchBroom__BaseMapView__) */
