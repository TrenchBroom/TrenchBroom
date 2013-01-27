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

#ifndef __TrenchBroom__AngleEditor__
#define __TrenchBroom__AngleEditor__

#include "View/SmartPropertyEditor.h"

#include "GL/glew.h"

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class SharedResources;
    }
    
    namespace View {
        class AngleEditorCanvas : public wxGLCanvas {
        public:
            typedef std::vector<float> AngleList;
        private:
            static const unsigned int CircleSegments = 32;

            Renderer::SharedResources& m_sharedResources;
            Renderer::Camera& m_mapCamera;
            wxGLContext* m_glContext;
            
            AngleList m_angles;
        public:
            AngleEditorCanvas(wxWindow* parent, Renderer::SharedResources& sharedResources, Renderer::Camera& mapCamera);

            inline void setAngles(const AngleList& angles) {
                m_angles = angles;
            }
            
            void OnPaint(wxPaintEvent& event);
            
            DECLARE_EVENT_TABLE()
        };
        
        class AngleEditor : public SmartPropertyEditor {
        private:
            wxPanel* m_panel;
            AngleEditorCanvas* m_canvas;
        protected:
            virtual wxWindow* createVisual(wxWindow* parent);
            virtual void destroyVisual();
            virtual void updateVisual();
        public:
            AngleEditor(SmartPropertyEditorManager& manager);
        };
    }
}

#endif /* defined(__TrenchBroom__AngleEditor__) */
