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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextureView.h"

#include "Color.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/FaceTexture.h"
#include "GL/GL.h"
#include "Renderer/RenderResources.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vbo.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        TextureView::TextureView(wxWindow* parent, wxWindowID windowId, Renderer::RenderResources& resources) :
        wxGLCanvas(parent, windowId, &resources.glAttribs().front(), wxDefaultPosition, wxDefaultSize),
        m_resources(resources),
        m_glContext(new wxGLContext(this, resources.sharedContext())),
        m_texture(NULL) {
            Bind(wxEVT_PAINT, &TextureView::OnPaint, this);
        }
        
        TextureView::~TextureView() {
            if (m_glContext != NULL) {
                wxDELETE(m_glContext);
                m_glContext = NULL;
            }
        }

        void TextureView::setTexture(Assets::FaceTexture* texture) {
            m_texture = texture;
            Refresh();
        }

        void TextureView::OnPaint(wxPaintEvent& event) {
            if (!IsShownOnScreen())
                return;

            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                
                glEnable(GL_MULTISAMPLE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDisable(GL_DEPTH_TEST);
                glFrontFace(GL_CCW);
                
                glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), backgroundColor.w());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                if (m_texture != NULL) {
                    const float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
                    const float viewTop       = static_cast<float>(GetClientRect().GetBottom());
                    const float viewRight     = static_cast<float>(GetClientRect().GetRight());
                    const float viewBottom    = static_cast<float>(GetClientRect().GetTop());
                    const float viewWidth     = viewRight - viewLeft;
                    const float viewHeight    = viewBottom - viewTop;
                    
                    const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
                    const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY) * translationMatrix(Vec3f(0.0f, 0.0f, 0.1f));
                    const Renderer::Transformation transformation(projection, view);
                    
                    float scale;
                    if (m_texture->width() >= m_texture->height())
                        scale = m_texture->width() <= viewWidth ? 1.0f : viewWidth / m_texture->width();
                    else
                        scale = m_texture->height() <= viewHeight ? 1.0f : viewHeight / m_texture->height();
                    
                    const float texLeft = viewLeft + (viewWidth - m_texture->width() * scale) / 2.0f;
                    const float texRight = texLeft + m_texture->width() * scale;
                    const float texBottom = viewTop + (viewHeight - m_texture->height() * scale) / 2.0f;
                    const float texTop = texBottom + m_texture->height() * scale;

                    typedef Renderer::VertexSpecs::P2T2::Vertex TextureVertex;
                    TextureVertex::List vertices(4);
                    vertices[0] = TextureVertex(Vec2f(texLeft, texBottom),  Vec2f(0.0f, 0.0f));
                    vertices[1] = TextureVertex(Vec2f(texLeft, texTop),     Vec2f(0.0f, 1.0f));
                    vertices[2] = TextureVertex(Vec2f(texRight, texTop),    Vec2f(1.0f, 1.0f));
                    vertices[3] = TextureVertex(Vec2f(texRight, texBottom), Vec2f(1.0f, 0.0f));
                    
                    Renderer::ActiveShader shader(m_resources.shaderManager(), Renderer::Shaders::TextureBrowserShader);
                    shader.set("ApplyTinting", false);
                    shader.set("Texture", 0);
                    shader.set("Brightness", prefs.getFloat(Preferences::Brightness));
                    shader.set("GrayScale", m_texture->overridden());
                    
                    Renderer::Vbo vbo(0xFF);
                    Renderer::SetVboState setVboState(vbo);
                    setVboState.mapped();
                    
                    Renderer::VertexArray vertexArray(vbo, GL_QUADS, vertices);
                    m_texture->activate();
                    
                    setVboState.active();
                    vertexArray.render();
                }

                SwapBuffers();
            }
        }
    }
}
