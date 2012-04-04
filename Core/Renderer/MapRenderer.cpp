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

#include "MapRenderer.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

namespace TrenchBroom {
    
    RenderContext::RenderContext() {
        backgroundColor.x = 0;
        backgroundColor.y = 0;
        backgroundColor.z = 0;
        backgroundColor.w = 1;
        renderOrigin = true;
        originAxisLength = 64;
    }

    void MapRenderer::render(RenderContext& context) {
        glClearColor(context.backgroundColor.x, context.backgroundColor.y, context.backgroundColor.z, context.backgroundColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_FLAT);
        
        if (context.renderOrigin) {
            glBegin(GL_LINES);
            glColor4f(1, 0, 0, 0.5f);
            glVertex3f(-context.originAxisLength, 0, 0);
            glVertex3f(context.originAxisLength, 0, 0);
            glColor4f(0, 1, 0, 0.5f);
            glVertex3f(0, -context.originAxisLength, 0);
            glVertex3f(0, context.originAxisLength, 0);
            glColor4f(0, 0, 1, 0.5f);
            glVertex3f(0, 0, -context.originAxisLength);
            glVertex3f(0, 0, context.originAxisLength);
            glEnd();
        }
    }
    
}