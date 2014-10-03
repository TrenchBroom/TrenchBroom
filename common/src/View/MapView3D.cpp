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

#include "MapView3D.h"
#include "Logger.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "View/CameraTool.h"
#include "View/MapDocument.h"
#include "View/MovementRestriction.h"

namespace TrenchBroom {
    namespace View {
        MapView3D::MapView3D(wxWindow* parent, Logger* logger, MapDocumentWPtr document, Renderer::MapRenderer& renderer) :
        RenderView(parent, attribs()),
        m_logger(logger),
        m_document(document),
        m_movementRestriction(new MovementRestriction()),
        m_vbo(new Renderer::Vbo(0xFFFFFFF)),
        m_renderer(renderer),
        m_camera(),
        m_compass(new Renderer::Compass(*m_movementRestriction)),
        m_toolBox(this, this),
        m_cameraTool(NULL) {
            createTools();
        }

        MapView3D::~MapView3D() {
            destroyTools();
            delete m_compass;
            delete m_vbo;
            delete m_movementRestriction;
        }
        
        void MapView3D::doInitializeGL() {
            const wxString vendor   = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            const wxString renderer = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            const wxString version  = wxString::FromUTF8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
            
            m_logger->info(wxString::Format(L"Renderer info: %s version %s from %s", renderer, version, vendor));
            m_logger->info("Depth buffer bits: %d", depthBits());
            
            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
        }
        
        void MapView3D::doUpdateViewport(const int x, const int y, const int width, const int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }
        
        bool MapView3D::doShouldRenderFocusIndicator() const {
            return true;
        }
        
        void MapView3D::doRender() {
            MapDocumentSPtr document = lock(m_document);
            Renderer::RenderContext renderContext(m_camera, contextHolder()->fontManager(), contextHolder()->shaderManager());
            setupGL(renderContext);
            
            Renderer::RenderBatch renderBatch(*m_vbo);
            
            renderMap(renderContext, renderBatch);
            renderCompass(renderBatch);
            
            renderBatch.render(renderContext);
        }

        void MapView3D::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }

        void MapView3D::renderMap(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_renderer.render(renderContext, renderBatch);
        }

        void MapView3D::renderCompass(Renderer::RenderBatch& renderBatch) {
            m_compass->render(renderBatch);
        }

        Ray3 MapView3D::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits MapView3D::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }
        
        void MapView3D::doShowPopupMenu() {
        }

        void MapView3D::createTools() {
            m_cameraTool = new CameraTool(m_document, m_camera);
            m_toolBox.addTool(m_cameraTool);
        }
        
        void MapView3D::destroyTools() {
        }

        const GLContextHolder::GLAttribs& MapView3D::attribs() {
            static bool initialized = false;
            static GLContextHolder::GLAttribs attribs;
            if (initialized)
                return attribs;
            
            int testAttribs[] =
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
            
            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }
            
            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }
        
        int MapView3D::depthBits() {
            return attribs()[3];
        }
        
        bool MapView3D::multisample() {
            return attribs()[4] != 0;
        }
    }
}
