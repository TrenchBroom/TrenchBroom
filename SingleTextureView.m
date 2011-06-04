//
//  SingleTextureView.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SingleTextureView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"

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
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            
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
