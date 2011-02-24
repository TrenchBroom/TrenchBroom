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
#import "GLFontManager.h"
#import "GLFont.h"
#import "GLString.h"
#import "Map.h"

@implementation TextureView

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];

    Texture* texture = [layout textureAt:clickPoint];
    if (texture != nil)
        [target textureSelected:texture];
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

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glPolygonMode(GL_FRONT, GL_FILL);
    
    NSArray* rows = [layout rowsInY:NSMinY(visibleRect) height:NSHeight(visibleRect)];
    NSEnumerator* rowEn = [rows objectEnumerator];
    TextureViewLayoutRow* row;
    
    NSSet* usedTextureNames = [map textureNames];
    NSFont* font = [NSFont systemFontOfSize:12];
    GLFont* glFont = [fontManager glFontFor:font];
    
    while ((row = [rowEn nextObject])) {
        NSArray* cells = [row cells];
        NSEnumerator* cellEn = [cells objectEnumerator];
        TextureViewLayoutCell* cell;
        while ((cell = [cellEn nextObject])) {
            float tx = [cell textureRect].origin.x;
            float ty = [cell textureRect].origin.y;
            float tx2 = tx + [cell textureRect].size.width;
            float ty2 = ty + [cell textureRect].size.height;
            
            Texture* texture = [cell texture];
            [texture activate];
            
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(tx, ty, 0);
            glTexCoord2f(0, 1);
            glVertex3f(tx, ty2, 0);
            glTexCoord2f(1, 1);
            glVertex3f(tx2, ty2, 0);
            glTexCoord2f(1, 0);
            glVertex3f(tx2, ty, 0);
            glEnd();
            
            [texture deactivate];
            
            if (selectedTextureNames != nil && [selectedTextureNames containsObject:[texture name]]) {
                glColor4f(0.6, 0, 0, 1);
                glBegin(GL_LINE_LOOP);
                glVertex3f(tx - 0.5, ty - 0.5, 0);
                glVertex3f(tx - 0.5, ty2 + 0.5, 0);
                glVertex3f(tx2 + 0.5, ty2 + 0.5, 0);
                glVertex3f(tx2 + 0.5, ty - 0.5, 0);
                glEnd();
            } else if ([usedTextureNames containsObject:[texture name]]) {
                glColor4f(0.6, 0.6, 0, 1);
                glBegin(GL_LINE_LOOP);
                glVertex3f(tx - 0.5, ty - 0.5, 0);
                glVertex3f(tx - 0.5, ty2 + 0.5, 0);
                glVertex3f(tx2 + 0.5, ty2 + 0.5, 0);
                glVertex3f(tx2 + 0.5, ty - 0.5, 0);
                glEnd();
            }
            
            glColor4f(1, 1, 1, 1);
            float nx = [cell nameRect].origin.x;
            float ny = [cell nameRect].origin.y;
            
            GLString* glString = [glStrings objectForKey:[texture name]];
            if (glString == nil) {
                glString = [glFont glStringFor:[texture name]];
                [glStrings setObject:glString forKey:[texture name]];
            }
            glPushMatrix();
            glTranslatef(nx, ny, 0);
            [glString render];
            glPopMatrix();
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

- (void)switchToContext:(NSOpenGLContext *)theSharedContext textureManager:(TextureManager *)theTextureManager fontManager:(GLFontManager *)theFontManager map:(Map *)theMap {
    if (theSharedContext == nil)
        [NSException raise:NSInvalidArgumentException format:@"shared context must not be nil"];
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    if (theFontManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"font manager must not be nil"];
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];

    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:theSharedContext];
    [self setOpenGLContext:sharingContext];
    [sharingContext release];

    [fontManager release];
    [fontManager = theFontManager retain];
    
    [textureManager release];
    textureManager = [theTextureManager retain];
    
    [map release];
    map = [theMap retain];

    if (glStrings != nil) {
        NSEnumerator* stringEn = [glStrings objectEnumerator];
        GLString* string;
        while ((string = [stringEn nextObject]))
            [string dispose];
        [glStrings release];
    }
    glStrings = [[NSMutableDictionary alloc] init];
    
    [layout release];
    
    NSFont* font = [NSFont systemFontOfSize:12];
    GLFont* glFont = [fontManager glFontFor:font];
    
    layout = [[TextureViewLayout alloc] initWithWidth:[self bounds].size.width innerMargin:10 outerMargin:5 font:glFont];
    [layout addTextures:[textureManager textures]];
    
    [self setNeedsDisplay:YES];
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter {
    [layout setTextureFilter:theFilter];
    [self reshape];
    [self setNeedsDisplay:YES];
}

- (void)setSelectedTextureNames:(NSSet *)theNames {
    [selectedTextureNames release];
    selectedTextureNames = [theNames retain];
    [self setNeedsDisplay:YES];
}

- (void)dealloc {
    if (glStrings != nil) {
        NSEnumerator* stringEn = [glStrings objectEnumerator];
        GLString* string;
        while ((string = [stringEn nextObject]))
            [string dispose];
        [glStrings release];
    }

    [layout release];
    [textureManager release];
    [fontManager release];
    [map release];
    [selectedTextureNames release];
    [super dealloc];
}

@end
