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

#ifndef __TrenchBroom__ToolView__
#define __TrenchBroom__ToolView__

#include "Hit.h"
#include "Renderer/GL.h"
#include "View/InputState.h"

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderContext;
    }

    namespace View {
        class AnimationManager;
        class Tool;
        
        class ToolView : public wxGLCanvas {
        public:
            typedef std::vector<int> GLAttribs;
        protected:
            Renderer::Camera& m_camera;
            InputState m_inputState;
        private:
            wxGLContext* m_glContext;
            bool m_initialized;

            Tool* m_toolChain;
            Tool* m_dragReceiver;
            Tool* m_modalReceiver;
            Tool* m_dropReceiver;
            Tool* m_savedDropReceiver;
            
            wxPoint m_clickPos;
            bool m_ignoreNextDrag;
            bool m_ignoreNextClick;
            wxDateTime m_lastFrameActivation;

            AnimationManager* m_animationManager;
        protected:
            ToolView(wxWindow* parent, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext = NULL);
        public:
            virtual ~ToolView();
            
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

            const wxGLContext* glContext() const;
        
            bool anyToolActive() const;
        protected:
            void addTool(Tool* tool);
            bool toolActive(const Tool* tool) const;
            void toggleTool(Tool* tool);
            void deactivateAllTools();
            void cancelCurrentDrag();
            
            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderTools(Renderer::RenderContext& renderContext);

            void updateHits();

            void resetCamera();
        public:
            void animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration);
        private:
            void bindEvents();

            void initializeGL();
            void updateViewport();
            void render();
            
            ModifierKeyState modifierKeys();
            bool updateModifierKeys();
            bool clearModifierKeys();
            MouseButtonState mouseButton(wxMouseEvent& event);
            
            void showPopupMenu();
        private:
            virtual void doInitializeGL();
            virtual void doUpdateViewport(int x, int y, int width, int height) = 0;
            virtual void doRender() = 0;
            virtual void doShowPopupMenu();
            
            virtual void doResetCamera() = 0;
            virtual Hits doGetHits(const Ray3d& pickRay) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ToolView__) */
