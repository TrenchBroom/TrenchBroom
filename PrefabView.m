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

#import "PrefabView.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Math.h"
#import "PrefabManager.h"
#import "Prefab.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "math.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"
#import "Camera.h"
#import "PrefabGroup.h"
#import "PrefabLayout.h"
#import "PrefabLayoutGroupRow.h"
#import "PrefabLayoutPrefabCell.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "PrefabViewTarget.h"
#import "VertexData.h"
#import "GLUtils.h"
#import "PrefabNameSheetController.h"

@interface PrefabView (private)

- (BOOL)isCameraModifierPressed:(NSEvent *)event;
- (void)resetCamera:(Camera *)camera forPrefab:(id <Prefab>)prefab;
- (void)addPrefab:(id <Prefab>)prefab;
- (void)prefabAdded:(NSNotification *)notification;
- (void)prefabRemoved:(NSNotification *)notification;
- (void)prefabGroupChanged:(NSNotification *)notification;
- (void)selectPrefab:(id <Prefab>)thePrefab;

@end

@implementation PrefabView (private)

- (BOOL)isCameraModifierPressed:(NSEvent *)event {
    return ([event modifierFlags] & NSShiftKeyMask) != 0;
}

- (void)resetCamera:(Camera *)camera forPrefab:(id <Prefab>)prefab {
    TBoundingBox* maxBounds = [prefab maxBounds];
    TVector3f s, p, d, u;
    sizeOfBounds(maxBounds, &s);
    
    scaleV3f(&s, 0.5f, &p);
    addV3f(&p, [prefab center], &p);
    
    subV3f([prefab center], &p, &d);
    
    crossV3f(&d, &ZAxisPos, &u);
    crossV3f(&u, &d, &u);
    
    normalizeV3f(&d, &d);
    normalizeV3f(&u, &u);
    
    [camera moveTo:&p];
    [camera setDirection:&d up:&u];
}

- (void)addPrefab:(id <Prefab>)prefab {
    Camera* camera = [[Camera alloc] initWithFieldOfVision:90 nearClippingPlane:10 farClippingPlane:1000];
    [self resetCamera:camera forPrefab:prefab];
    
    [cameras setObject:camera forKey:[prefab prefabId]];
    [camera release];
    
    [layout invalidate];
    [self selectPrefab:prefab];
}

- (void)prefabAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Prefab> prefab = [userInfo objectForKey:PrefabKey];
    [self addPrefab:prefab];
}

- (void)prefabRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Prefab> prefab = [userInfo objectForKey:PrefabKey];
    if (selectedPrefab == prefab)
        [self selectPrefab:nil];
    
    [cameras removeObjectForKey:[prefab prefabId]];
    [layout invalidate];
    [self setNeedsDisplay:YES];
}

- (void)prefabGroupChanged:(NSNotification *)notification {
    [layout invalidate];
    [self setNeedsDisplay:YES];
}

- (void)selectPrefab:(id <Prefab>)thePrefab {
    if (selectedPrefab != thePrefab) {
        selectedPrefab = thePrefab;
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PrefabSelectionDidChange object:self];
        
        [self setNeedsDisplay:YES];
    }
}

@end

@implementation PrefabView

- (id)initWithCoder:(NSCoder *)aDecoder {
    if ((self = [super initWithCoder:aDecoder])) {
        prefabsPerRow = 1;
        cameras = [[NSMutableDictionary alloc] init];
        
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        for (id <PrefabGroup> group in [prefabManager prefabGroups])
            for (id <Prefab> prefab in [group prefabs])
                [self addPrefab:prefab];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(prefabAdded:) name:PrefabAdded object:prefabManager];
        [center addObserver:self selector:@selector(prefabRemoved:) name:PrefabRemoved object:prefabManager];
        [center addObserver:self selector:@selector(prefabGroupChanged:) name:PrefabGroupAdded object:prefabManager];
        [center addObserver:self selector:@selector(prefabGroupChanged:) name:PrefabGroupRemoved object:prefabManager];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [cameras release];
    [layout release];
    [glResources release];
    [fontManager release];
    [super dealloc];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    if (action == @selector(insertPrefabIntoMap:)) {
        return selectedPrefab != nil;
    } else if (action == @selector(renamePrefab:)) {
        return selectedPrefab != nil && ![selectedPrefab readOnly];
    } else if (action == @selector(deletePrefab:)) {
        return selectedPrefab != nil && ![selectedPrefab readOnly];
    }
    
    return NO;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPointFromBase:[theEvent locationInWindow]];
    [self selectPrefab:[layout prefabAt:clickPoint]];
    
    if ([theEvent clickCount] == 1 && [self isCameraModifierPressed:theEvent]) {
        draggedPrefab = selectedPrefab;
    } else if ([theEvent clickCount] == 2) {
        if (selectedPrefab != nil)
            [target prefabSelected:selectedPrefab];
    }
}

- (void)rightMouseDown:(NSEvent *)theEvent {
    NSPoint clickPoint = [self convertPointFromBase:[theEvent locationInWindow]];
    [self selectPrefab:[layout prefabAt:clickPoint]];
    [super rightMouseDown:theEvent];
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
        
        [[self openGLContext] makeCurrentContext];
        float h =  fmaxf([layout height], NSHeight([[self superview] bounds]));
        
        [[self superview] setNeedsDisplay:YES];
        [self setFrameSize:NSMakeSize(NSWidth(frame), h)];
        [self setNeedsDisplay:YES];
    }
}

- (void)renderFace:(id <Face>)face {
    glBegin(GL_POLYGON);
    const TVertexList* vertices = [face vertices];
    for (int i = 0; i < vertices->count; i++)
        glVertexV3f(&vertices->items[i]->position);
    glEnd();
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    for (PrefabLayoutGroupRow* groupRow in [layout groupRows]) {
        for (PrefabLayoutPrefabCell* cell in [groupRow cells]) {
            id <Prefab> prefab = [cell prefab];
            NSRect prefabBounds = [cell prefabBounds];
            NSRect cameraBounds = NSMakeRect(NSMinX(prefabBounds), NSHeight(visibleRect) - NSMaxY(prefabBounds) + NSMinY(visibleRect), NSWidth(prefabBounds), NSHeight(prefabBounds));
            
            Camera* camera = [cameras objectForKey:[prefab prefabId]];
            [camera updateView:cameraBounds];
            
            for (id <Entity> entity in [prefab entities]) {
                for (id <Brush> brush in [entity brushes]) {
                    for (id <Face> face in [brush faces]) {
                        glEnable(GL_TEXTURE_2D);
                        glPolygonMode(GL_FRONT, GL_FILL);
                        glColor4f(0, 0, 0, 1);
                        [self renderFace:face];
                        
                        glDisable(GL_TEXTURE_2D);
                        glSetEdgeOffset(0.5f);
                        glPolygonMode(GL_FRONT, GL_LINE);
                        if (selectedPrefab == prefab)
                            glColor4f(1, 0, 0, 1);
                        else
                            glColor4f(1, 1, 1, 0.5f);
                        [self renderFace:face];
                        glResetEdgeOffset();
                    }
                }
            }
        }
    }

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
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    [fontManager activate];
    glTranslatef(0, 2 * NSMinY(visibleRect), 0);
    
    for (PrefabLayoutGroupRow* groupRow in [layout groupRows]) {
        GLString* groupName = [groupRow name];
        
        glPushMatrix();
        NSRect titleBounds = [groupRow titleBounds];
        NSRect titleBarBounds = [groupRow titleBarBounds];

        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        /*
        glRectf(NSMinX(titleBarBounds),  NSHeight(visibleRect) - NSMinY(titleBarBounds), NSMaxX(titleBarBounds), NSHeight(visibleRect) - NSMaxY(titleBarBounds));

         */
        glTranslatef(NSMinX(titleBounds),  NSHeight(visibleRect) - NSMaxY(titleBounds), 0);
        glColor4f(1, 1, 1, 1);
        [groupName render];
        glPopMatrix();

        for (PrefabLayoutPrefabCell* cell in [groupRow cells]) {
            id <Prefab> prefab = [cell prefab];
            GLString* prefabName = [cell name];
            
            glPushMatrix();
            NSRect nameBounds = [cell nameBounds];
            glTranslatef(NSMinX(nameBounds),  NSHeight(visibleRect) - NSMaxY(nameBounds), 0);
            
            if (selectedPrefab == prefab)
                glColor4f(1, 0, 0, 1);
            else
                glColor4f(1, 1, 1, 1);
            [prefabName render];
            glPopMatrix();
        }
    }

    [fontManager deactivate];
    
    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGLResources {
    if (glResources == theGLResources)
        return;
    
    [self selectPrefab:nil];
    
    [glResources release];
    glResources = [theGLResources retain];
    
    [layout release];
    [fontManager release];
    
    if (glResources != nil) {
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext release];
    } else {
        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:nil];
        [self setOpenGLContext:context];
        [context release];
    }

    [[self openGLContext] makeCurrentContext];
    
    fontManager = [[GLFontManager alloc] init];
    NSFont* font = [NSFont systemFontOfSize:13];
    PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
    layout = [[PrefabLayout alloc] initWithPrefabManager:prefabManager prefabsPerRow:prefabsPerRow fontManager:fontManager font:font];

    [self reshape];
}

- (void)setTarget:(id <PrefabViewTarget>)theTarget {
    target = theTarget;
}

- (void)setPrefabsPerRow:(int)thePrefabsPerRow {
    prefabsPerRow = thePrefabsPerRow;
    [layout setPrefabsPerRow:prefabsPerRow];
    [self reshape];
}

- (IBAction)insertPrefabIntoMap:(id)sender {
    if (selectedPrefab != nil)
        [target prefabSelected:selectedPrefab];
}

- (id<Prefab>)selectedPrefab {
    return selectedPrefab;
}

@end
