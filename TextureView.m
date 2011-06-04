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
#import "Texture.h"
#import "TextureViewLayout.h"
#import "TextureViewLayoutRow.h"
#import "TextureViewLayoutCell.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "GLResources.h"

@implementation TextureView

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (layout != nil) {
        NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
        
        Texture* texture = [layout textureAt:clickPoint];
        if (texture != nil)
            [target textureSelected:texture];
    }
}


- (void)drawRect:(NSRect)dirtyRect {
    [layout layout];
    
    NSRect visibleRect = [self visibleRect];
    glViewport(0, 0, NSWidth(visibleRect), NSHeight(visibleRect));
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(NSMinX(visibleRect), 
               NSMaxX(visibleRect), 
               NSMinY(visibleRect), 
               NSMaxY(visibleRect));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
    
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glTranslatef(0, 2 * NSMinY(visibleRect), 0);
    if (layout != nil) {
        NSArray* rows = [layout rowsInY:NSMinY(visibleRect) height:NSHeight(visibleRect)];
        NSEnumerator* rowEn = [rows objectEnumerator];
        TextureViewLayoutRow* row;

        GLFontManager* fontManager = [glResources fontManager];
        NSFont* font = [NSFont systemFontOfSize:12];
        
        [fontManager activate];
        
        while ((row = [rowEn nextObject])) {
            NSArray* cells = [row cells];
            NSEnumerator* cellEn = [cells objectEnumerator];
            TextureViewLayoutCell* cell;
            while ((cell = [cellEn nextObject])) {
                float tx = NSMinX([cell textureRect]);
                float ty = NSHeight(visibleRect) - NSMinY([cell textureRect]);
                float tx2 = NSMaxX([cell textureRect]);
                float ty2 = NSHeight(visibleRect) - NSMaxY([cell textureRect]);
                
                glEnable(GL_TEXTURE_2D);
                Texture* texture = [cell texture];
                [texture activate];
                
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(tx, ty, 0);
                glTexCoord2f(1, 0);
                glVertex3f(tx2, ty, 0);
                glTexCoord2f(1, 1);
                glVertex3f(tx2, ty2, 0);
                glTexCoord2f(0, 1);
                glVertex3f(tx, ty2, 0);
                glEnd();
                
                [texture deactivate];
                glDisable(GL_TEXTURE_2D);
                
                if (selectedTextureNames != nil && [selectedTextureNames containsObject:[texture name]]) {
                    glColor4f(0.6, 0, 0, 1);
                    glBegin(GL_LINE_LOOP);
                    glVertex3f(tx  - 0.5, ty  - 0.5, 0);
                    glVertex3f(tx2 + 0.5, ty  - 0.5, 0);
                    glVertex3f(tx2 + 0.5, ty2 + 0.5, 0);
                    glVertex3f(tx  - 0.5, ty2 + 0.5, 0);
                    glEnd();
                    /*
                } else if ([texture usageCount] > 0) {
                    glColor4f(0.6, 0.6, 0, 1);
                    glBegin(GL_LINE_LOOP);
                    glVertex3f(tx  - 0.5, ty  - 0.5, 0);
                    glVertex3f(tx2 + 0.5, ty  - 0.5, 0);
                    glVertex3f(tx2 + 0.5, ty2 + 0.5, 0);
                    glVertex3f(tx  - 0.5, ty2 + 0.5, 0);
                    glEnd();
                     */
                }
                
                glColor4f(1, 1, 1, 1);
                float nx = NSMinX([cell nameRect]);
                float ny = NSHeight(visibleRect) - NSMaxY([cell nameRect]);
                
                GLString* glString = [fontManager glStringFor:[texture name] font:font];
                glPushMatrix();
                glTranslatef(nx, ny, 0);
                [glString render];
                glPopMatrix();
            }
        }
        [fontManager deactivate];
    }
    
    
    [[self openGLContext] flushBuffer];
}

-  (void)reshape {
    if (layout != nil) {
        NSRect frame = [self frame];
        [layout setWidth:NSWidth(frame)];
        [layout layout];
        
        float h =  fmaxf([layout height], NSHeight([[self superview] bounds]));
        
        [[self superview] setNeedsDisplay:YES];
        [self setFrameSize:NSMakeSize(NSWidth(frame), h)];
        [self setNeedsDisplay:YES];
    }
}

- (void)textureManagerChanged:(NSNotification *)notification {
    [layout clear];
    
    TextureManager* textureManager = [glResources textureManager];
    [layout addTextures:[textureManager texturesByCriterion:sortCriterion]];
    [self reshape];
}

- (void)setGLResources:(GLResources *)theGLResources {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

    if (glResources != nil) {
        TextureManager* textureManager = [glResources textureManager];
        [center removeObserver:self name:TextureManagerChanged object:textureManager];

        [glResources release];
        glResources = nil;

        [layout clear];
    }
    glResources = [theGLResources retain];
    
    if (glResources != nil) {
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext release];

        TextureManager* textureManager = [glResources textureManager];
        if (layout == nil) {
            GLFontManager* fontManager = [glResources fontManager];
            NSFont* font = [NSFont systemFontOfSize:12];
        
            layout = [[TextureViewLayout alloc] initWithWidth:[self bounds].size.width innerMargin:10 outerMargin:5 fontManager:fontManager font:font];
        } else {
            [layout clear];
        }
        
        [layout addTextures:[textureManager texturesByCriterion:sortCriterion]];
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TextureManagerChanged object:textureManager];
    }
    
    [self setNeedsDisplay:YES];
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter {
    if (layout != nil) {
        [layout setTextureFilter:theFilter];
        [self reshape];
    }
}

- (void)setSortCriterion:(ETextureSortCriterion)criterion {
    sortCriterion = criterion;
    [layout clear];
    
    TextureManager* textureManager = [glResources textureManager];
    [layout addTextures:[textureManager texturesByCriterion:sortCriterion]];
    [self reshape];
}

- (void)setSelectedTextureNames:(NSSet *)theNames {
    [selectedTextureNames release];
    selectedTextureNames = [theNames retain];
    [self setNeedsDisplay:YES];
}

- (void)dealloc {
    [selectedTextureNames release];
    [super dealloc];
}

@end
