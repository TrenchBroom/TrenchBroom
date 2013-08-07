/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__

#include "Controller/Command.h"
#include "GL/GL.h"
#include "Renderer/Camera.h"
#include "Renderer/FontManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/ShaderManager.h"
#include "View/InputState.h"
#include "View/ViewTypes.h"

#include <vector>
#include <wx/event.h>
#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Controller {
        class ControllerFacade;
    }
    
    namespace View {
        class BaseTool;
        class CameraTool;
        class SelectionTool;
        class Logger;
        
        class MapView : public wxGLCanvas {
        private:
            Logger* m_logger;
            bool m_initialized;
            wxGLContext* m_glContext;
            
            View::MapDocumentPtr m_document;
            Controller::ControllerFacade& m_controller;
            Renderer::Camera m_camera;
            Renderer::FontManager m_fontManager;
            Renderer::MapRenderer m_renderer;
            Renderer::ShaderManager m_shaderManager;
            
            InputState m_inputState;
            wxPoint m_clickPos;
            CameraTool* m_cameraTool;
            SelectionTool* m_selectionTool;
            BaseTool* m_toolChain;
            BaseTool* m_dragReceiver;
        public:
            MapView(wxWindow* parent, Logger* logger, View::MapDocumentPtr document, Controller::ControllerFacade& controller);
            ~MapView();
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(wxFocusEvent& event);
            void OnKillFocus(wxFocusEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);

            void commandDo(Controller::Command::Ptr command);
            void commandDone(Controller::Command::Ptr command);
            void commandDoFailed(Controller::Command::Ptr command);
            void commandUndo(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
            void commandUndoFailed(Controller::Command::Ptr command);
        private:
            void createTools();
            void deleteTools();
            void bindEvents();
            void initializeGL();
            
            static const int* attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
