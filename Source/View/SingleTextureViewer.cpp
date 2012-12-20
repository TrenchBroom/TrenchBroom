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
#include "Renderer/SharedResources.h"
#include "Renderer/TextureRenderer.h"
#include "Renderer/TextureRendererManager.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <wx/wx.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(SingleTextureViewer, wxGLCanvas)
        EVT_PAINT(SingleTextureViewer::OnPaint)
        END_EVENT_TABLE()
        
        SingleTextureViewer::SingleTextureViewer(wxWindow* parent, Renderer::SharedResources& sharedResources) :
        wxGLCanvas(parent, wxID_ANY, sharedResources.attribs(), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
        m_textureRendererManager(sharedResources.textureRendererManager()),
        m_glContext(NULL),
        m_texture(NULL) {
            m_glContext = new wxGLContext(this, sharedResources.sharedContext());
        }

        SingleTextureViewer::~SingleTextureViewer() {
            if (m_glContext != NULL) {
                wxDELETE(m_glContext);
                m_glContext = NULL;
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
                    Renderer::TextureRenderer& textureRenderer = m_textureRendererManager.renderer(m_texture);
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
                    textureRenderer.activate();
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
                    textureRenderer.deactivate();
                }
                
				SwapBuffers();
            }
        }
    }
}
