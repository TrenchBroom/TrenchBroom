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

#include "EditorView.h"

#include "Model/MapDocument.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "View/EditorFrame.h"
#include "View/MapGLCanvas.h"

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(EditorView, wxView);
        
        EditorView::EditorView() : wxView(), m_camera(NULL), m_renderer(NULL) {
            EditorFrame* frame = new EditorFrame();
            m_console = new Utility::Console(frame->logView());
            SetFrame(frame);
            frame->Show();
        }
        
        bool EditorView::OnCreate(wxDocument* doc, long flags) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float fieldOfVision = prefs.getFloat(Preferences::CameraFieldOfVision);
            float nearPlane = prefs.getFloat(Preferences::CameraNearPlane);
            float farPlane = prefs.getFloat(Preferences::CameraFarPlane);
            Vec3f position(0.0f, 0.0f, 0.0f);
            Vec3f direction(1.0f, 0.0f, 0.0f);
            m_camera = new Renderer::Camera(fieldOfVision, nearPlane, farPlane, position, direction);
            
            Model::MapDocument* document = static_cast<Model::MapDocument*>(doc);
            m_renderer = new Renderer::MapRenderer(document->map());
            m_renderer->loadMap();
            
            EditorFrame* frame = static_cast<EditorFrame*>(GetFrame());
            frame->mapCanvas()->Initialize(*m_camera, *m_renderer);
            
            return wxView::OnCreate(doc, flags);
        }
        
        void EditorView::OnUpdate(wxView* sender, wxObject* hint) {
            m_renderer->loadMap();
        }
        
        void EditorView::OnDraw(wxDC* dc) {
        }
        
        bool EditorView::OnClose(bool deleteWindow) {
            if (deleteWindow) {
                wxWindow* frame = GetFrame();
                SetFrame(NULL);
                wxDELETE(frame);
            }
            
            delete m_camera;
            m_camera = NULL;
            delete m_renderer;
            m_renderer = NULL;
            return wxView::OnClose(deleteWindow);
        }

    }
}