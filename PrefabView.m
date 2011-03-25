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
#import "Brush.h"
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
#import "PrefabGroup.h"
#import "PrefabLayout.h"
#import "PrefabLayoutGroupRow.h"
#import "PrefabLayoutPrefabCell.h"
#import "GLFontManager.h"
#import "GLFont.h"
#import "GLString.h"

@implementation PrefabView

- (void)resetCamera:(Camera *)camera forPrefab:(id <Prefab>)prefab {
    BoundingBox* maxBounds = [prefab maxBounds];
    Vector3f* size = [maxBounds size];

    Vector3f* p = [[Vector3f alloc] initWithFloatVector:size];
    [p scale:0.5f];
    [p add:[prefab center]];
    
    Vector3f* d = [[Vector3f alloc] initWithFloatVector:[prefab center]];
    [d sub:p];
    
    Vector3f* u = [[Vector3f alloc] initWithFloatVector:d];
    [u cross:[Vector3f zAxisPos]]; // points to right
    [u cross:d];
    
    [camera moveTo:p];
    [camera setDirection:d up:u];

    [p release];
    [d release];
    [u release];
}

- (void)addPrefab:(id <Prefab>)prefab {
    Camera* camera = [[Camera alloc] initWithFieldOfVision:90 nearClippingPlane:10 farClippingPlane:1000];
    [self resetCamera:camera forPrefab:prefab];

    [cameras setObject:camera forKey:[prefab prefabId]];
    [camera release];
    
    [layout invalidate];
    [self setNeedsDisplay:YES];
}

- (void)prefabAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Prefab> prefab = [userInfo objectForKey:PrefabKey];
    [self addPrefab:prefab];
}

- (void)prefabRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Prefab> prefab = [userInfo objectForKey:PrefabKey];
    
    [cameras removeObjectForKey:[prefab prefabId]];
    [layout invalidate];
    [self setNeedsDisplay:YES];
}

- (void)prefabGroupChanged:(NSNotification *)notification {
    [layout invalidate];
    [self setNeedsDisplay:YES];
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    if (self = [super initWithCoder:aDecoder]) {
        prefabsPerRow = 1;
        glStrings = [[NSMutableDictionary alloc] init];
        cameras = [[NSMutableDictionary alloc] init];
        
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        NSEnumerator* groupEn = [[prefabManager prefabGroups] objectEnumerator];
        id <PrefabGroup> group;
        while ((group = [groupEn nextObject])) {
            NSEnumerator* prefabEn = [[group prefabs] objectEnumerator];
            id <Prefab> prefab;
            while ((prefab = [prefabEn nextObject]))
                [self addPrefab:prefab];
        }
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(prefabAdded:) name:PrefabAdded object:prefabManager];
        [center addObserver:self selector:@selector(prefabRemoved:) name:PrefabRemoved object:prefabManager];
        [center addObserver:self selector:@selector(prefabGroupChanged:) name:PrefabGroupAdded object:prefabManager];
        [center addObserver:self selector:@selector(prefabGroupChanged:) name:PrefabGroupRemoved object:prefabManager];
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
    if (layout != nil) {
        NSRect frame = [self frame];
        [layout setWidth:NSWidth(frame)];
        
        float h =  fmaxf([layout height], NSHeight([[self superview] bounds]));
        
        [[self superview] setNeedsDisplay:YES];
        [self setFrameSize:NSMakeSize(NSWidth(frame), h)];
        [self setNeedsDisplay:YES];
    }
}

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSShiftKeyMask) != 0;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if ([theEvent clickCount] == 1 && [self isCameraModifierPressed:theEvent]) {
        draggedPrefab = [layout prefabAt:clickPoint];
    } else if ([theEvent clickCount] == 2) {
        id <Prefab> prefab = [layout prefabAt:clickPoint];
        if (prefab != nil)
            [target prefabSelected:prefab];
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (draggedPrefab != nil) {
        Camera* camera = [cameras objectForKey:[draggedPrefab prefabId]];
        [camera orbitCenter:[draggedPrefab center] hAngle:[theEvent deltaX] / 70 vAngle:[theEvent deltaY] / 70];
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (draggedPrefab != nil) {
        Camera* camera = [cameras objectForKey:[draggedPrefab prefabId]];
        [self resetCamera:camera forPrefab:draggedPrefab];
        draggedPrefab = nil;
        [self setNeedsDisplay:YES];
    }
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
    Vertex* vertex;
    glBegin(GL_POLYGON);
    while ((vertex = [vertexEn nextObject])) {
        if (texture != nil) {
            [face texCoords:t forVertex:[vertex vector]];
            glTexCoord2f([t x] / width, [t y] / height);
        }
        
        glVertex3f([[vertex vector] x], [[vertex vector] y], [[vertex vector] z]);
        
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
    
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_2D);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    NSEnumerator* groupRowEn = [[layout groupRows] objectEnumerator];
    PrefabLayoutGroupRow* groupRow;
    while ((groupRow = [groupRowEn nextObject])) {
        NSEnumerator* cellEn = [[groupRow cells] objectEnumerator];
        PrefabLayoutPrefabCell* cell;
        while ((cell = [cellEn nextObject])) {
            id <Prefab> prefab = [cell prefab];
            NSRect prefabBounds = [cell prefabBounds];
            NSRect cameraBounds = NSMakeRect(NSMinX(prefabBounds), NSHeight(visibleRect) - NSMinY(prefabBounds) - NSHeight(prefabBounds) + NSMinY(visibleRect), NSWidth(prefabBounds), NSHeight(prefabBounds));
            
            Camera* camera = [cameras objectForKey:[prefab prefabId]];
            [camera updateView:cameraBounds];
            
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
        }
    }
    
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT, GL_FILL);

    glViewport(0, 0, NSWidth(visibleRect), NSHeight(visibleRect));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(NSMinX(visibleRect), 
               NSMaxX(visibleRect), 
               NSMaxY(visibleRect), 
               NSMinY(visibleRect));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
    
    glDisable(GL_DEPTH_TEST);

    groupRowEn = [[layout groupRows] objectEnumerator];
    while ((groupRow = [groupRowEn nextObject])) {
        id <PrefabGroup> prefabGroup = [groupRow prefabGroup];
        GLString* groupNameString = [glStrings objectForKey:[prefabGroup prefabGroupId]];
        if (groupNameString == nil) {
            GLFont* glFont = [layout glFont];
            groupNameString = [glFont glStringFor:[prefabGroup name]];
            [glStrings setObject:groupNameString forKey:[prefabGroup prefabGroupId]];
        }
        
        glPushMatrix();
        NSRect titleBounds = [groupRow titleBounds];
        NSRect titleBarBounds = [groupRow titleBarBounds];

        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        glRectf(NSMinX(titleBarBounds), NSMinY(titleBarBounds), NSMaxX(titleBarBounds), NSMaxY(titleBarBounds));
        
        glTranslatef(NSMinX(titleBounds), NSMinY(titleBounds), 0);
        glColor4f(1, 1, 1, 1);
        [groupNameString render];
        glPopMatrix();
        
        NSEnumerator* cellEn = [[groupRow cells] objectEnumerator];
        PrefabLayoutPrefabCell* cell;
        while ((cell = [cellEn nextObject])) {
            id <Prefab> prefab = [cell prefab];
            GLString* prefabNameString = [glStrings objectForKey:[prefab prefabId]];
            if (prefabNameString == nil) {
                GLFont* glFont = [layout glFont];
                prefabNameString = [glFont glStringFor:[prefab name]];
                [glStrings setObject:prefabNameString forKey:[prefab prefabId]];
            }
            
            glPushMatrix();
            NSRect nameBounds = [cell nameBounds];
            glTranslatef(NSMinX(nameBounds), NSMinY(nameBounds), 0);
            glColor4f(1, 1, 1, 1);
            [prefabNameString render];
            glPopMatrix();
        }
    }

    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGLResources {
    [glResources release];
    glResources = [theGLResources retain];
    
    if (glResources != nil) {
        [glStrings removeAllObjects];
        
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext makeCurrentContext];
        [sharingContext release];

        [layout release];

        GLFontManager* fontManager = [glResources fontManager];
        NSFont* font = [NSFont systemFontOfSize:13];
        GLFont* glFont = [fontManager glFontFor:font];
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];

        layout = [[PrefabLayout alloc] initWithPrefabManager:prefabManager prefabsPerRow:prefabsPerRow glFont:glFont];
                  
    }

    [self reshape];
}

- (void)setPrefabsPerRow:(int)thePrefabsPerRow {
    prefabsPerRow = thePrefabsPerRow;
    [layout setPrefabsPerRow:prefabsPerRow];
    [self reshape];
}
- (void)dealloc {
    [glStrings release];
    [cameras release];
    [glResources release];
    [layout release];
    [super dealloc];
}

@end
