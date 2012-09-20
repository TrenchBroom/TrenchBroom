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

#include "SingleTextureViewer.h"

#include "GL/Capabilities.h"
#include "Model/Texture.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <wx/wx.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(SingleTextureViewer, wxGLCanvas)
        EVT_PAINT(SingleTextureViewer::OnPaint)
        END_EVENT_TABLE()

        int* SingleTextureViewer::Attribs() {
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
            
            return m_attribs;
        }
        
        SingleTextureViewer::SingleTextureViewer(wxWindow* parent, wxGLContext* sharedContext) :
        wxGLCanvas(parent, wxID_ANY, Attribs(), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
        m_glContext(NULL),
        m_texture(NULL) {
            m_glContext = new wxGLContext(this, sharedContext);
            delete [] m_attribs;
            m_attribs = NULL;
        }

        SingleTextureViewer::~SingleTextureViewer() {
            if (m_glContext != NULL) {
                wxDELETE(m_glContext);
                m_glContext = NULL;
            }
            if (m_attribs != NULL) {
                delete m_attribs;
                m_attribs = NULL;
            }
        }

        void SingleTextureViewer::setTexture(Model::Texture* texture) {
            m_texture = texture;
            Refresh();
        }

        void SingleTextureViewer::OnPaint(wxPaintEvent& event) {
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
				Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
				const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
				glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (m_texture != NULL) {
                    wxRect bounds = GetRect();
                    float viewLeft      = static_cast<float>(bounds.GetLeft());
                    float viewTop       = static_cast<float>(bounds.GetTop());
                    float viewRight     = static_cast<float>(bounds.GetRight());
                    float viewBottom    = static_cast<float>(bounds.GetBottom());
                    float viewWidth = viewRight - viewLeft;
                    float viewHeight = viewBottom - viewTop;
                    
                    Mat4f projection;
                    projection.setOrtho(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
                    
                    Mat4f view;
                    view.setView(Vec3f::NegZ, Vec3f::PosY);
                    view.translate(Vec3f(0.0f, 0.0f, 0.1f));
                    
                    glMatrixMode(GL_PROJECTION);
                    glLoadMatrixf(projection.v);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadMatrixf(view.v);
                    
                    float texLeft, texTop, texRight, texBottom;
                    float scale;
                    if (m_texture->width() >= m_texture->height())
                        scale = m_texture->width() <= viewWidth ? 1.0f : viewWidth / m_texture->width();
                    else
                        scale = m_texture->height() <= viewHeight ? 1.0f : viewHeight / m_texture->height();
                    
                    texLeft = viewLeft + (viewWidth - m_texture->width() * scale) / 2.0f;
                    texRight = texLeft + m_texture->width() * scale;
                    texBottom = viewTop + (viewHeight - m_texture->height() * scale) / 2.0f;
                    texTop = texBottom + m_texture->height() * scale;

                    glEnable(GL_TEXTURE_2D);
                    m_texture->activate();
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f, 0.0f);
                    glVertex3f(texLeft, texBottom, 0.0f);
                    glTexCoord2f(1.0f, 0.0f);
                    glVertex3f(texRight, texBottom, 0.0f);
                    glTexCoord2f(1.0f, 1.0f);
                    glVertex3f(texRight, texTop, 0.0f);
                    glTexCoord2f(0.0f, 1.0f);
                    glVertex3f(texLeft, texTop, 0.0f);
                    glEnd();
                    m_texture->deactivate();
                }
                
				SwapBuffers();
            }
        }
    }
}