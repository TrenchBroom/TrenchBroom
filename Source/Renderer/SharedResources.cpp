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

#include "SharedResources.h"

#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/Palette.h"
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Text/FontManager.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"

#include <GL/glew.h>
#include <wx/glcanvas.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace Renderer {
        IMPLEMENT_DYNAMIC_CLASS(SharedResources, wxFrame)
        
        BEGIN_EVENT_TABLE(SharedResources, wxFrame)
        EVT_IDLE(SharedResources::OnIdle)
        END_EVENT_TABLE()

        SharedResources::SharedResources() :
        wxFrame(NULL, wxID_ANY, wxT("TrenchBroom Render Resources"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLIP_CHILDREN | wxFRAME_NO_TASKBAR),
        m_palette(NULL),
        m_modelRendererManager(NULL),
        m_shaderManager(NULL),
        m_textureRendererManager(NULL),
        m_fontManager(NULL),
        m_attribs(NULL),
        m_multisample(false),
        m_samples(0),
        m_depthbits(0),
        m_sharedContext(NULL),
        m_glCanvas(NULL),
        m_retainCount(0) {}

        SharedResources::SharedResources(Model::TextureManager& textureManager, Utility::Console& console) :
        wxFrame(NULL, wxID_ANY, wxT("TrenchBroom Render Resources"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLIP_CHILDREN | wxFRAME_NO_TASKBAR),
        m_palette(NULL),
        m_modelRendererManager(NULL),
        m_shaderManager(NULL),
        m_textureRendererManager(NULL),
        m_fontManager(NULL),
        m_attribs(NULL),
        m_multisample(false),
        m_samples(0),
        m_depthbits(0),
        m_sharedContext(NULL),
        m_glCanvas(NULL),
        m_retainCount(0) {
            Create(textureManager, console);
        }
        
        void SharedResources::Create(Model::TextureManager& textureManager, Utility::Console& console) {
            int attribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };
            
            SetSize(0, 0);
            Show(true);
            Raise();
            
            size_t index = 0;
            while (m_attribs == NULL && attribs[index] != 0) {
                size_t count = 0;
                for (; attribs[index + count] != 0; count++);
                if (wxGLCanvas::IsDisplaySupported(&attribs[index])) {
                    m_attribs = new int[count + 1];
                    for (size_t i = 0; i < count; i++)
                        m_attribs[i] = attribs[index + i];
                    m_attribs[count] = 0;
                    m_depthbits = m_attribs[3];
                    if (m_attribs[4] > 0) {
                        m_multisample = true;
                        m_samples = m_attribs[7];
                    }
                }
                index += count + 1;
            }
            
            assert(m_attribs != NULL);
            
            m_glCanvas = new wxGLCanvas(this, wxID_ANY, m_attribs, wxDefaultPosition, GetClientSize());
            m_sharedContext = new wxGLContext(m_glCanvas);
            
            m_sharedContext->SetCurrent(*m_glCanvas);
            const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            console.info("Renderer info: %s version %s from %s", renderer, version, vendor);
            console.info("Depth buffer bits: %d", m_depthbits);
            
            if (m_multisample)
                console.info("Multisampling enabled");
            else
                console.info("Multisampling disabled");
            
            glewExperimental = GL_TRUE;
            GLenum glewState = glewInit();
            if (glewState != GLEW_OK)
                console.error("Unable to initialize glew: %s", glewGetErrorString(glewState));
            
            if (PointHandleRenderer::instancingSupported())
                console.info("OpenGL instancing enabled");
            else
                console.info("OpenGL instancing disabled");
            
            m_modelRendererManager = new EntityModelRendererManager(console);
            m_shaderManager = new ShaderManager(console);
            m_textureRendererManager = new TextureRendererManager(textureManager);
            m_fontManager = new Text::FontManager(console);
            
            SetPosition(wxPoint(-10, -10));
            Hide();
        }

        SharedResources::~SharedResources() {
            if (m_sharedContext != NULL && m_glCanvas != NULL)
                m_sharedContext->SetCurrent(*m_glCanvas);

            delete [] m_attribs;
            m_attribs = NULL;
            delete m_fontManager;
            m_fontManager = NULL;
            delete m_shaderManager;
            m_shaderManager = NULL;
            delete m_textureRendererManager;
            m_textureRendererManager = NULL;
            delete m_modelRendererManager;
            m_modelRendererManager = NULL;
			delete m_palette;
			m_palette = NULL;
            wxDELETE(m_sharedContext);
        }

        void SharedResources::loadPalette(const String& palettePath) {
            if (m_palette != NULL)
                delete m_palette;
            m_palette = new Palette(palettePath);

            m_modelRendererManager->setPalette(*m_palette);
            m_textureRendererManager->setPalette(*m_palette);
        }

        void SharedResources::OnIdle(wxIdleEvent& event) {
            SetPosition(wxPoint(-10, -10));
            Hide();
            event.Skip();
        }
    }
}
