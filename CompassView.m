/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "CompassView.h"
#import "Camera.h"
#import "CompassFigure.h"
#import "Math.h"

@interface CompassView (private)

- (void)cameraChanged:(NSNotification *)notification;

@end

@implementation CompassView (private)

- (void)cameraChanged:(NSNotification *)notification {
    [self setNeedsDisplay:YES];
}

@end

@implementation CompassView

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [compassFigure release];
    [super dealloc];
}

- (void)setCamera:(Camera *)theCamera {
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    [compassFigure release];
    camera = theCamera;
    compassFigure = [[CompassFigure alloc] initWithCamera:camera];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self];
    [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
}

- (void)drawRect:(NSRect)dirtyRect {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (camera != nil) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        
        NSRect viewport = [self visibleRect];
        glViewport(NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport));
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-NSWidth(viewport) / 2, NSWidth(viewport) / 2, -NSHeight(viewport) / 2, NSHeight(viewport) / 2, 1, 100);
        
        TVector3f position;
        const TVector3f* direction = [camera direction];
        const TVector3f* up = [camera up];
        
        scaleV3f(direction, -25, &position);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(position.x,
                  position.y,
                  position.z,
                  position.x + direction->x,
                  position.y + direction->y,
                  position.z + direction->z,
                  up->x,
                  up->y,
                  up->z);
        
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        
        glEnable(GL_LIGHT0);
        GLfloat ambientLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };
        GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
        GLfloat specularLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
        GLfloat lightPosition[] = { [camera position]->x, [camera position]->y, [camera position]->z, 1.0f };
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        
        float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
        glMateriali(GL_FRONT, GL_SHININESS, 96);
        
        glScalef(1.9f, 1.9f, 1.9f);
        [compassFigure render];
        
        glDisable(GL_LIGHT0);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
    }
    
    [[self openGLContext] flushBuffer];
}

@end
