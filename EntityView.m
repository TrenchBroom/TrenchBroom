//
//  EntityView.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityView.h"
#import "Math.h"
#import "Camera.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"
#import "GLResources.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "EntityDefinitionLayout.h"
#import "EntityDefinitionLayoutCell.h"
#import "EntityRendererManager.h"
#import "EntityRenderer.h"

@implementation EntityView

- (void)resetCamera:(Camera *)camera forEntityDefinition:(EntityDefinition *)entityDefinition {
    TBoundingBox* maxBounds = [entityDefinition maxBounds];
    TVector3f s, p, d, u, c;
    centerOfBounds(maxBounds, &c);
    sizeOfBounds(maxBounds, &s);
    
    scaleV3f(&s, 0.5f, &p);
    addV3f(&p, &c, &p);
    
    subV3f(&c, &p, &d);
    
    crossV3f(&d, &ZAxisPos, &u);
    crossV3f(&u, &d, &u);
    
    normalizeV3f(&d, &d);
    normalizeV3f(&u, &u);
    
    [camera moveTo:&p];
    [camera setDirection:&d up:&u];
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    if ((self = [super initWithCoder:aDecoder])) {
        cameras = [[NSMutableDictionary alloc] init];
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
        
        [[self openGLContext] makeCurrentContext];
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
        draggedEntityDefinition = [layout entityDefinitionAt:clickPoint];
    } else if ([theEvent clickCount] == 2) {
        EntityDefinition* entityDefinition = [layout entityDefinitionAt:clickPoint];
        if (entityDefinition != nil)
            [target entityDefinitionSelected:entityDefinition];
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (draggedEntityDefinition != nil) {
        Camera* camera = [cameras objectForKey:[draggedEntityDefinition name]];
        
        TVector3f center;
        centerOfBounds([draggedEntityDefinition bounds], &center);
        
        [camera orbitCenter:&center hAngle:[theEvent deltaX] / 70 vAngle:[theEvent deltaY] / 70];
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (draggedEntityDefinition != nil) {
        Camera* camera = [cameras objectForKey:[draggedEntityDefinition name]];
        [self resetCamera:camera forEntityDefinition:draggedEntityDefinition];
        draggedEntityDefinition = nil;
        [self setNeedsDisplay:YES];
    }
}



- (void)drawRect:(NSRect)dirtyRect {
    NSRect visibleRect = [self visibleRect];
    
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (glResources != nil) {
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glShadeModel(GL_FLAT);
        glEnable(GL_TEXTURE_2D);
        
        EntityRendererManager* entityRendererManager = [glResources entityRendererManager];
        [entityRendererManager activate];
        
        TVector3i origin = {0, 0, 0};
        
        NSEnumerator* rowEn = [[layout rows] objectEnumerator];
        NSArray* row;
        while ((row = [rowEn nextObject])) {
            NSEnumerator* cellEn = [row objectEnumerator];
            EntityDefinitionLayoutCell* cell;
            while ((cell = [cellEn nextObject])) {
                EntityDefinition* definition = [cell entityDefinition];
                NSRect definitionBounds = [cell entityDefinitionBounds];

                NSRect cameraBounds = NSMakeRect(NSMinX(definitionBounds), NSHeight(visibleRect) - NSMaxY(definitionBounds) + NSMinY(visibleRect), NSWidth(definitionBounds), NSHeight(definitionBounds));
                
                Camera* camera = [cameras objectForKey:[definition name]];
                [camera updateView:cameraBounds];
                
                id <EntityRenderer> renderer = [entityRendererManager entityRendererForDefinition:definition];
                glPushMatrix();
                [renderer renderAtOrigin:&origin angle:nil];
                glPopMatrix();
            }
        }
        
        [entityRendererManager deactivate];
        
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
        
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        
        GLFontManager* fontManager = [glResources fontManager];
        [fontManager activate];
        
        glTranslatef(0, 2 * NSMinY(visibleRect), 0);
        
        rowEn = [[layout rows] objectEnumerator];
        while ((row = [rowEn nextObject])) {
            NSEnumerator* cellEn = [row objectEnumerator];
            EntityDefinitionLayoutCell* cell;
            while ((cell = [cellEn nextObject])) {
                GLString* nameString = [cell nameString];

                glPushMatrix();
                NSRect nameBounds = [cell nameBounds];
                glTranslatef(NSMinX(nameBounds),  NSHeight(visibleRect) - NSMaxY(nameBounds), 0);
                glColor4f(1, 1, 1, 1);
                [nameString render];
                glPopMatrix();
            }
        }
        
        [fontManager deactivate];
    }
    
    [[self openGLContext] flushBuffer];
}

- (void)setGLResources:(GLResources *)theGLResources entityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager {
    [glResources release];
    glResources = [theGLResources retain];
    
    [cameras removeAllObjects];
    [entityDefinitionManager release];
    entityDefinitionManager = [theEntityDefinitionManager retain];
    
    if (glResources != nil && entityDefinitionManager != nil) {
        NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[self pixelFormat] shareContext:[glResources openGLContext]];
        [self setOpenGLContext:sharingContext];
        [sharingContext release];
        
        [layout release];
        
        GLFontManager* fontManager = [glResources fontManager];
        NSFont* font = [NSFont systemFontOfSize:13];
        layout = [[EntityDefinitionLayout alloc] initWithEntityDefinitionManager:entityDefinitionManager fontManager:fontManager font:font];
        
        NSEnumerator* definitionEn = [[entityDefinitionManager definitionsOfType:EDT_POINT] objectEnumerator];
        EntityDefinition* definition;
        while ((definition = [definitionEn nextObject])) {
            Camera* camera = [[Camera alloc] initWithFieldOfVision:90 nearClippingPlane:10 farClippingPlane:1000];
            [self resetCamera:camera forEntityDefinition:definition];
            [cameras setObject:camera forKey:[definition name]];
            [camera release];
        }
    }
    
    [self reshape];
}

- (void)dealloc {
    [cameras release];
    [glResources release];
    [entityDefinitionManager release];
    [layout release];
    [super dealloc];
}

@end
