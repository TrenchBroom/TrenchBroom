//
//  Renderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Renderer.h"
#import <OpenGL/glu.h>
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "GeometryLayer.h"
#import "SelectionLayer.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "RenderContext.h"
#import "SelectionManager.h"
#import "TrackingManager.h"
#import "Brush.h"
#import "Face.h"
#import "MutableFace.h"
#import "Camera.h"
#import "Vector3f.h"
#import "Options.h"
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"
#import "Edge.h"

NSString* const RendererChanged = @"RendererChanged";

@interface Renderer (private)

- (void)addFace:(id <Face>)face;
- (void)removeFace:(id <Face>)face;
- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;
- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)faceDidChange:(NSNotification *)notification;
- (void)brushWillChange:(NSNotification *)notification;
- (void)brushDidChange:(NSNotification *)notification;
- (void)brushAdded:(NSNotification *)notification;
- (void)brushRemoved:(NSNotification *)notification;
- (void)entityAdded:(NSNotification *)notification;
- (void)entityRemoved:(NSNotification *)notification;

- (void)selectionAdded:(NSNotification *)notification;
- (void)selectionRemoved:(NSNotification *)notification;
- (void)trackedObjectChanged:(NSNotification *)notification;
- (void)cameraChanged:(NSNotification *)notification;
- (void)optionsChanged:(NSNotification *)notification;

@end

@implementation Renderer (private)

- (void)addFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer addFace:face includeEdges:NO];
    else 
        [geometryLayer addFace:face includeEdges:NO];
}

- (void)removeFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer removeFace:face includeEdges:NO];
    else
        [geometryLayer removeFace:face includeEdges:NO];
}

- (void)addBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];
    
    SelectionManager* selectionManager = [windowController selectionManager];

    NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject])) {
        if ([selectionManager isBrushSelected:brush] || 
            [selectionManager isFaceSelected:[edge leftFace]] || 
            [selectionManager isFaceSelected:[edge rightFace]])
            [selectionLayer addEdge:edge];
        else
            [geometryLayer addEdge:edge];
    }
}

- (void)removeBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    
    NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject])) {
        if ([selectionManager isBrushSelected:brush] || 
            [selectionManager isFaceSelected:[edge leftFace]] || 
            [selectionManager isFaceSelected:[edge rightFace]])
            [selectionLayer removeEdge:edge];
        else
            [geometryLayer removeEdge:edge];
    }
}

- (void)addEntity:(id <Entity>)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self addBrush:brush];
}

- (void)removeEntity:(id <Entity>)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];
}

- (void)faceWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer removeFace:face includeEdges:YES];
    else
        [geometryLayer removeFace:face includeEdges:YES];
}

- (void)faceDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer addFace:face includeEdges:YES];
    else
        [geometryLayer addFace:face includeEdges:YES];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    
    [self removeBrush:brush];
}

- (void)brushDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];

    [self addBrush:brush];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    [self addBrush:brush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    [self removeBrush:brush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    [self addEntity:entity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entityRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    [self removeEntity:entity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                [geometryLayer removeFace:face includeEdges:YES];
                [selectionLayer addFace:face includeEdges:YES];
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [geometryLayer removeFace:face includeEdges:YES];
            [selectionLayer addFace:face includeEdges:YES];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                [selectionLayer removeFace:face includeEdges:YES];
                [geometryLayer addFace:face includeEdges:YES];
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [selectionLayer removeFace:face includeEdges:YES];
            [geometryLayer addFace:face includeEdges:YES];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)trackedObjectChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id untrackedObject = [userInfo objectForKey:UntrackedObjectKey];
    id trackedObject = [userInfo objectForKey:TrackedObjectKey];
    
    
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)optionsChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

@end

@implementation Renderer

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];

        geometryLayer = [[GeometryLayer alloc] initWithWindowController:windowController];
        selectionLayer = [[SelectionLayer alloc] initWithWindowController:windowController];

        MapDocument* map = [windowController document];

        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        
        [center addObserver:self selector:@selector(entityAdded:) name:EntityAdded object:map];
        [center addObserver:self selector:@selector(entityRemoved:) name:EntityRemoved object:map];
        [center addObserver:self selector:@selector(brushAdded:) name:BrushAdded object:map];
        [center addObserver:self selector:@selector(brushRemoved:) name:BrushRemoved object:map];
        [center addObserver:self selector:@selector(brushWillChange:) name:BrushWillChange object:map];
        [center addObserver:self selector:@selector(brushDidChange:) name:BrushDidChange object:map];
        [center addObserver:self selector:@selector(faceDidChange:) name:FaceDidChange object:map];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
        TrackingManager* trackingManager = [windowController trackingManager];
        [center addObserver:self selector:@selector(trackedObjectChanged:) name:TrackedObjectChanged object:trackingManager];
        
        Camera* camera = [windowController camera];
        [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
        
        Options* options = [windowController options];
        [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
    }
    
    return self;
}

- (void)render {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Options* options = [windowController options];
    
    RenderContext* renderContext = [[RenderContext alloc] initWithOptions:options];
    [geometryLayer render:renderContext];
    [selectionLayer render:renderContext];
    
    [renderContext release];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [windowController release];
    [geometryLayer release];
    [selectionLayer release];
    [super dealloc];
}

@end
