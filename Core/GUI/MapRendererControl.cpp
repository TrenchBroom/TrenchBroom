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


#include "MapRendererControl.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#include "Gwen/Structures.h"

namespace TrenchBroom {
    namespace Gui {
        MapRendererControl::MapRendererControl(Base* parent) : Base(parent) {
        }
        
        MapRendererControl::~MapRendererControl() {
        }
        
        void MapRendererControl::Render(Skin::Base* skin) {
            const Gwen::Rect& bounds = GetBounds();
            float fov = 90;
            float near = 0.1f;
            float far = 2000;
            Vec3f position(-64, -64, 64);
            Vec3f direction(XAxisPos);
            Vec3f up(ZAxisPos);
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT | GL_CLIENT_PIXEL_STORE_BIT);
            
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            float vfrustum = tan(fov * M_PI / 360) * 0.75 * near;
            float hfrustum = vfrustum * bounds.w / (float)bounds.h;
            glFrustum(-hfrustum, hfrustum, -vfrustum, vfrustum, near, far);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glViewport(bounds.x, bounds.y, bounds.w, bounds.h);
            gluLookAt(position.x,
                      position.y,
                      position.z,
                      position.x + direction.x,
                      position.y + direction.y,
                      position.z + direction.z,
                      up.x,
                      up.y,
                      up.z);
            
            Renderer::RenderContext context;
            m_mapRenderer.render(context);

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            
            glPopClientAttrib();
            glPopAttrib();
        }
    }
}