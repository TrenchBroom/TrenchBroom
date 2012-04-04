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

#import "DocumentView.h"
#import "MapRenderer.h"
#import "VecMath.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

using namespace TrenchBroom;

@implementation DocumentView

- (void)awakeFromNib {
    renderer = new MapRenderer();
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect viewport = [self visibleRect];
    glViewport(NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport));

    float fov = 90;
    float near = 0.1f;
    float far = 2000;
    Vec3f position(-64, -64, 64);
    Vec3f direction(XAxisPos);
    Vec3f up(ZAxisPos);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float vfrustum = tan(fov * M_PI / 360) * 0.75 * near;
    float hfrustum = vfrustum * NSWidth(viewport) / NSHeight(viewport);
    glFrustum(-hfrustum, hfrustum, -vfrustum, vfrustum, near, far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(position.x,
              position.y,
              position.z,
              position.x + direction.x,
              position.y + direction.y,
              position.z + direction.z,
              up.x,
              up.y,
              up.z);

    RenderContext context;
    MapRenderer* r = (MapRenderer *)renderer;
    r->render(context);
    
    [[self openGLContext] flushBuffer];
}

@end
