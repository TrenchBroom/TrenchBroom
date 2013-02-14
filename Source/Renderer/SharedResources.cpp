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

#include "GL/Capabilities.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/Palette.h"
#include "Renderer/TextureRendererManager.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Text/StringManager.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"

#include <GL/glew.h>
#include <wx/glcanvas.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace Renderer {
        SharedResources::SharedResources(Model::TextureManager& textureManager, Utility::Console& console) :
        wxFrame(NULL, wxID_ANY, wxT("TrenchBroom Render Resources")),
        m_palette(NULL),
        m_modelRendererManager(NULL),
        m_textureRendererManager(NULL),
        m_stringManager(NULL),
        m_attribs(NULL),
        m_sharedContext(NULL),
        m_glCanvas(NULL) {
            GL::Capabilities capabilities = GL::glCapabilities();
            if (capabilities.multisample) {
                m_attribs = new int[9];
                m_attribs[0] = WX_GL_RGBA;
                m_attribs[1] = WX_GL_DOUBLEBUFFER;
                m_attribs[2] = WX_GL_SAMPLE_BUFFERS;
                m_attribs[3] = 1;
                m_attribs[4] = WX_GL_SAMPLES;
                m_attribs[5] = capabilities.samples;
                m_attribs[6] = WX_GL_DEPTH_SIZE;
                m_attribs[7] = capabilities.depthBits;
                m_attribs[8] = 0;
            } else {
                m_attribs = new int[5];
                m_attribs[0] = WX_GL_RGBA;
                m_attribs[1] = WX_GL_DOUBLEBUFFER;
                m_attribs[2] = WX_GL_DEPTH_SIZE;
                m_attribs[3] = capabilities.depthBits;
                m_attribs[4] = 0;
            }

            m_glCanvas = new wxGLCanvas(this, wxID_ANY, m_attribs);
            m_sharedContext = new wxGLContext(m_glCanvas);

            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_glCanvas, 1, wxEXPAND);
            SetSizer(sizer);

            SetSize(100, 100);
            SetPosition(wxPoint(-110, -110));
            Show();
            Raise();

            m_sharedContext->SetCurrent(*m_glCanvas);
            const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            console.info("Renderer info: %s version %s from %s", renderer, version, vendor);

            console.info("Depth buffer bits: %d", capabilities.depthBits);

            if (capabilities.multisample)
                console.info("Multisampling enabled");
            else
                console.info("Multisampling disabled");

            GLenum glewState = glewInit();
            if (glewState != GLEW_OK)
                console.error("Unable to initialize glew: %s", glewGetErrorString(glewState));

            if (GLEW_ARB_draw_instanced && GLEW_ARB_texture_float)
                console.info("OpenGL instancing enabled");
            else
                console.info("OpenGL instancing disabled");

            m_modelRendererManager = new EntityModelRendererManager(console);
            m_shaderManager = new ShaderManager(console);
            m_textureRendererManager = new TextureRendererManager(textureManager);
            m_stringManager = new Text::StringManager(console);

            Hide();
        }

        SharedResources::~SharedResources() {
            if (m_sharedContext != NULL && m_glCanvas != NULL)
                m_sharedContext->SetCurrent(*m_glCanvas);

            delete m_attribs;
            m_attribs = NULL;
            delete m_stringManager;
            m_stringManager = NULL;
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

    }
}
