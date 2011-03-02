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
#import "Camera.h"
#import "MathCache.h"

@implementation PrefabView

- (void)resetCamera:(Camera *)camera forPrefab:(Prefab *)prefab {
    BoundingBox* maxBounds = [prefab maxBounds];
    Vector3f* size = [maxBounds size];

    MathCache* cache = [MathCache sharedCache];
    Vector3f* position = [cache vector3f];
    
    [position setFloat:size];
    [position scale:0.5f];
    [camera moveTo:position];
    [camera lookAt:[Vector3f nullVector]];
    
    [cache returnVector3f:position];
}

- (void)addPrefab:(Prefab *)prefab {
    Camera* camera = [[Camera alloc] initWithFieldOfVision:90 nearClippingPlane:10 farClippingPlane:1000];
    [self resetCamera:camera forPrefab:prefab];

    [cameras setObject:camera forKey:[prefab prefabId]];
    [camera release];
}

- (void)prefabAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Prefab* prefab = [userInfo objectForKey:PrefabKey];
    [self addPrefab:prefab];
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    if (self = [super initWithCoder:aDecoder]) {
        cameras = [[NSMutableDictionary alloc] init];

        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        NSEnumerator* prefabEn = [[prefabManager prefabs] objectEnumerator];
        Prefab* prefab;
        while ((prefab = [prefabEn nextObject]))
            [self addPrefab:prefab];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(prefabAdded:) name:PrefabAdded object:prefabManager];
    }
    
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)reshape {
    [super reshape];
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    NSRect visibleRect = [self visibleRect];
    float gridSize = NSWidth(visibleRect) / prefabsPerRow;
    
    int col = clickPoint.x / gridSize;
    int row = clickPoint.y / gridSize;
    int index = row * prefabsPerRow + col;
    
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
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (draggedPrefab != nil) {
        Camera* camera = [cameras objectForKey:[draggedPrefab prefabId]];
        [camera orbitCenter:[Vector3f nullVector] hAngle:[theEvent deltaX] / 70 vAngle:[theEvent deltaY] / 70];
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    Camera* camera = [cameras objectForKey:[draggedPrefab prefabId]];
    [self resetCamera:camera forPrefab:draggedPrefab];
    draggedPrefab = nil;
    [self setNeedsDisplay:YES];
}

- (void)renderFace:(id <Face>)face {
    Vector2f* t = [[Vector2f alloc] init];
    
    TextureManager* textureManager = [glResources textureManager];
    Texture* texture = [textureManager textureForName:[face texture]];
    if (texture != nil)
        [texture activate];
    
    float width = texture != nil ? [texture width] : 1;
    float height = texture != nil ? [texture height] : 1;
    
    NSEnumerator* vertexEn = [[face vertices] objectEnumerator];
    Vector3f* vertex;
    glBegin(GL_POLYGON);
    while ((vertex = [vertexEn nextObject])) {
        if (texture != nil) {
            [face texCoords:t forVertex:vertex];
            glTexCoord2f([t x] / width, [t y] / height);
        }
        
        glVertex3f([vertex x], [vertex y], [vertex z]);
        
    }
    glEnd();
    
    if (texture != nil)
        [texture deactivate];
    
    [t release];
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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

    float gridSize = NSWidth(visibleRect) / prefabsPerRow;
    int row = 0;
    int col = 0;

    NSEnumerator* prefabEn = [prefabs objectEnumerator];
    Prefab* prefab;
    while ((prefab = [prefabEn nextObject])) {
        int x = col * gridSize;
        float y = NSHeight(visibleRect) - (row + 1) * gridSize;
        
        Camera* camera = [cameras objectForKey:[prefab prefabId]];
        [camera updateView:NSMakeRect(x, y, gridSize, gridSize)];
        
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
                    [self renderFace:face];
                    
                    glDisable(GL_TEXTURE_2D);
                    glPolygonMode(GL_FRONT, GL_LINE);
                    glColor4f(1, 1, 1, 0.5);
                    [self renderFace:face];
                }
            }
        }
        col++;
        if (col == prefabsPerRow) {
            col = 0;
            row++;
        }
    }
    
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

- (void)setPrefabsPerRow:(int)thePrefabsPerRow {
    prefabsPerRow = thePrefabsPerRow;
    [self setNeedsDisplay:YES];
}
- (void)dealloc {
    [cameras release];
    [glResources release];
    [super dealloc];
}

@end
