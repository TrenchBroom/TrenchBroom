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

#import "SingleTextureView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"
#import "PreferencesManager.h"

@implementation SingleTextureView

+ (void)initialize {
    [self exposeBinding:@"textureName"];
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    glViewport(0, 0, NSWidth(visibleRect), NSHeight(visibleRect));
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(NSMinX(visibleRect), 
               NSMaxX(visibleRect), 
               NSMaxY(visibleRect), 
               NSMinY(visibleRect));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 0.1, 0, 0, -1, 0, 1, 0);
    
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

    if (glResources != nil && textureName != nil) {
        TextureManager* textureManager = [glResources textureManager];
        Texture* texture = [textureManager textureForName:textureName];
        
        if (texture != nil) {
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            
            PreferencesManager* preferences = [PreferencesManager sharedManager];
            float brightness = [preferences brightness];
            float color[3] = {brightness / 2, brightness / 2, brightness / 2};
            
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
            
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
            
            glPolygonMode(GL_FRONT, GL_FILL);
            
            [texture activate];
            
            float x1 = (NSWidth(visibleRect) - [texture width]) / 2;
            float y1 = (NSHeight(visibleRect) - [texture height]) / 2;
            float x2 = x1 + [texture width];
            float y2 = y1 + [texture height];
            
            
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(x1, y1, 0);
            glTexCoord2f(0, 1);
            glVertex3f(x1, y2, 0);
            glTexCoord2f(1, 1);
            glVertex3f(x2, y2, 0);
            glTexCoord2f(1, 0);
            glVertex3f(x2, y1, 0);
            glEnd();
            
            [texture deactivate];
        }
    }
    
    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGlResources {
    if (glResources != nil) {
        [glResources release];
    }
    
    glResources = [theGlResources retain];
    
    if (glResources != nil) {
        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:context];
        [context release];
    }
    
    [self setNeedsDisplay:YES];
}

- (void)setTextureName:(NSString *)theTextureName {
    if ([textureName isEqualToString:theTextureName])
        return;
    
    [textureName release];
    textureName = [theTextureName retain];
    
    [self setNeedsDisplay:YES];
}

- (void)dealloc {
    [glResources release];
    [textureName release];
    [super dealloc];
}
     
@end
