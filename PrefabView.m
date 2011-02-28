//
//  PrefabView.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "PrefabManager.h"
#import "Prefab.h"
#import "Entity.h"
#import "Face.h"
#import "Quaternion.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "BoundingBox.h"
#import "math.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"
#import "InspectorController.h"

static int NUM_COLS = 5;

@implementation PrefabView

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)reshape {
    [super reshape];

    NSRect visibleRect = [self visibleRect];
    gridSize = NSWidth(visibleRect) / NUM_COLS;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    int col = clickPoint.x / gridSize;
    int row = clickPoint.y / gridSize;
    int index = row * NUM_COLS + col;
    
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    NSArray* prefabs = [prefabManager prefabs];

    if (index < [prefabs count]) {
        if ([theEvent clickCount] == 1) {
            draggedPrefab = [prefabs objectAtIndex:index];
        } else {
            id windowController = [[self window] windowController];
            [windowController prefabSelected:[prefabs objectAtIndex:index]];
        }
    }
    
    vAngle = 0;
    hAngle = 0;
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (draggedPrefab != nil) {
        hAngle += ([theEvent deltaX] / 70);
        vAngle += ([theEvent deltaY] / 70);
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    draggedPrefab = nil;
    [self setNeedsDisplay:YES];
}

- (void)renderFace:(id <Face>)face rotation:(Quaternion *)rotation {
    Vector3f* v = [[Vector3f alloc] init];
    Vector2f* t = [[Vector2f alloc] init];
    
    TextureManager* textureManager = [glResources textureManager];
    Texture* texture = [textureManager textureForName:[face texture]];
    if (texture != nil)
        [texture activate];
    
    NSEnumerator* vertexEn = [[face vertices] objectEnumerator];
    Vector3f* vertex;
    glBegin(GL_POLYGON);
    while ((vertex = [vertexEn nextObject])) {
        if (texture != nil) {
            [face texCoords:t forVertex:vertex];
            glTexCoord2f([t x], [t y]);
        }
        
        [v setFloat:vertex];
        [rotation rotate:v];
        glVertex3f([v x], [v y], [v z]);
        
    }
    glEnd();
    
    if (texture != nil)
        [texture deactivate];
    
    [v release];
    [t release];
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    glViewport(0, 0, NSWidth(visibleRect), NSHeight(visibleRect));
    
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // glEnable(GL_TEXTURE_2D);
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    NSArray* prefabs = [prefabManager prefabs];

    int row = 0;
    int col = 0;

    Quaternion* p = [[Quaternion alloc] init];
    Quaternion* q = [[Quaternion alloc] init];
    Vector3f* v = [[Vector3f alloc] init];
    
    NSEnumerator* prefabEn = [prefabs objectEnumerator];
    Prefab* prefab;
    while ((prefab = [prefabEn nextObject])) {
        int x = col * gridSize;
        float y = NSHeight(visibleRect) - row * gridSize - gridSize;
        
        glViewport(x, y, gridSize, gridSize);
        
        BoundingBox* bounds = [prefab bounds];
        Vector3f* size = [bounds size];
        
        float width = fmax([size x], [size y]);
        float height = [size z];
        float dist = fmax(width, height) / 2;
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90, width / height, 10, 1000);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0,
                  -dist - 30,
                  0,
                  0,
                  0,
                  0,
                  0, 0, 1);
        
        if (prefab == draggedPrefab) {
            [p setAngle:hAngle axis:[Vector3f zAxisPos]];
            [v setFloat:[Vector3f xAxisPos]];
            [p rotate:v];
            [q setAngle:vAngle axis:v];
            [p mul:q];
        } else {
            [p setAngle:0 axis:[Vector3f nullVector]];
        }
        
        NSEnumerator* entityEn = [[prefab entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject])) {
                NSEnumerator* faceEn = [[brush faces] objectEnumerator];
                id <Face> face;
                while ((face = [faceEn nextObject])) {
                    glEnable(GL_TEXTURE_2D);
                    glPolygonMode(GL_FRONT, GL_FILL);
                    glColor4f(0, 0, 0, 1);
                    [self renderFace:face rotation:p];
                    
                    glDisable(GL_TEXTURE_2D);
                    glPolygonMode(GL_FRONT, GL_LINE);
                    glColor4f(1, 1, 1, 0.5);
                    [self renderFace:face rotation:p];
                }
            }
        }
        col++;
        if (col == NUM_COLS) {
            col = 0;
            row++;
        }
    }
    
    [q release];
    [p release];
    [v release];
    
    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGLResources {
    [glResources release];
    glResources = [theGLResources retain];
    
    if (glResources != nil) {
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext release];
    }

    [self setNeedsDisplay:YES];
}

- (void)dealloc {
    [glResources release];
    [super dealloc];
}

@end
