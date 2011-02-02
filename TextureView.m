//
//  TextureView.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "math.h"
#import "TextureManager.h"
#import "Texture.h"
#import "TextureViewLayout.h"
#import "TextureViewLayoutRow.h"
#import "TextureViewLayoutCell.h"

@implementation TextureView

- (void)awakeFromNib {
    [super awakeFromNib];

    /*
    NSOpenGLContext* sharedContext = [sharedView openGLContext];
    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:sharedContext];
    [self setOpenGLContext:sharingContext];
    [sharingContext release];
    
     */
    layout = [[TextureViewLayout alloc] initWithWidth:[self bounds].size.width innerMargin:10 outerMargin:5];
}

- (BOOL)isFlipped {
    return YES;
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
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glPolygonMode(GL_FRONT, GL_FILL);
    
    NSArray* rows = [layout rowsInY:NSMinY(visibleRect) height:NSHeight(visibleRect)];
//    NSArray* rows = [layout rows];
    NSEnumerator* rowEn = [rows objectEnumerator];
    TextureViewLayoutRow* row;
    
    while ((row = [rowEn nextObject])) {
        float y = [row y];
        
        NSArray* cells = [row cells];
        NSEnumerator* cellEn = [cells objectEnumerator];
        TextureViewLayoutCell* cell;
        while ((cell = [cellEn nextObject])) {
            float x = [cell x];
            float x2 = x + [cell textureWidth];
            float y2 = y + [cell textureHeight];
            
            Texture* texture = [cell texture];
            [texture activate];
            
            glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glTexCoord2f(0, 0);
            glVertex3f(x, y, 0);
            glTexCoord2f(0, 1);
            glVertex3f(x, y2, 0);
            glTexCoord2f(1, 1);
            glVertex3f(x2, y2, 0);
            glTexCoord2f(1, 0);
            glVertex3f(x2, y, 0);
            glEnd();
        }
    }
    [[self openGLContext] flushBuffer];
}

-  (void)reshape {
    NSRect frame = [self frame];
    [layout setWidth:NSWidth(frame)];
    
    float h =  fmaxf([layout height], NSHeight([[self superview] bounds]));
    
    [[self superview] setNeedsDisplay:YES];
    [self setFrameSize:NSMakeSize(NSWidth(frame), h)];
    [self setNeedsDisplay:YES];
}

- (void)setTextureManager:(TextureManager *)theManager {
    if (theManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];

    [textureManager release];
    textureManager = [theManager retain];
    
    [layout removeAllTextures];
    [layout addTextures:[textureManager textures]];
}

- (void)dealloc {
    [layout release];
    [textureManager release];
    [super dealloc];
}

@end
