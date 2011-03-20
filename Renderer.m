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

- (id <Figure>)figureForFace:(id <Face>)face;
- (void)addFace:(id <Face>)face;
- (void)removeFace:(id <Face>)face;
- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;
- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)faceChanged:(NSNotification *)notification;
- (void)faceAdded:(NSNotification *)notification;
- (void)faceRemoved:(NSNotification *)notification;
- (void)brushChanged:(NSNotification *)notification;
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

- (id <Figure>)figureForFace:(id <Face>)face {
    return (id <Figure>)face;
}

- (id <Figure>)figureForEdge:(Edge *)edge {
    return (id <Figure>)edge;
}

- (void)addFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    id <Figure> faceFigure = [self figureForFace:face];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer addFigure:faceFigure];
    else 
        [geometryLayer addFigure:faceFigure];
}

- (void)removeFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    id <Figure> faceFigure = [self figureForFace:face];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer removeFigure:faceFigure];
    else
        [geometryLayer removeFigure:faceFigure];
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
        id <Figure> edgeFigure = [self figureForEdge:edge];
        if ([selectionManager isBrushSelected:brush] || 
            [selectionManager isFaceSelected:[edge leftFace]] || 
            [selectionManager isFaceSelected:[edge rightFace]])
            [selectionLayer addFigure:edgeFigure];
        else
            [geometryLayer addFigure:edgeFigure];
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
        id <Figure> edgeFigure = [self figureForEdge:edge];
        if ([selectionManager isBrushSelected:brush] || 
            [selectionManager isFaceSelected:[edge leftFace]] || 
            [selectionManager isFaceSelected:[edge rightFace]])
            [selectionLayer removeFigure:edgeFigure];
        else
            [geometryLayer removeFigure:edgeFigure];
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

- (void)faceChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];
    id <Figure> faceFigure = [self figureForFace:face];
    if (faceFigure != nil) {
        [faceFigure invalidate];
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
            [selectionLayer invalidate];
        else
            [geometryLayer invalidate];
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)faceAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];
    [self addFace:face];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)faceRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];
    [self removeFace:face];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        id <Figure> figure = [self figureForFace:face];
        if (figure != nil) {
            [figure invalidate];
            SelectionManager* selectionManager = [windowController selectionManager];
            if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:brush])
                [selectionLayer invalidate];
            else
                [geometryLayer invalidate];
        }
    }
    
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
                id <Figure> figure = [self figureForFace:face];
                [geometryLayer removeFigure:figure];
                [selectionLayer addFigure:figure];
            }
            
            NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                id <Figure> figure = [self figureForEdge:edge];
                if (figure != nil) {
                    [geometryLayer removeFigure:figure];
                    [selectionLayer addFigure:figure];
                    [figure invalidate];
                }
            }
        }
    }
    
    if (faces != nil) {
        NSMutableSet* edges = [[NSMutableSet alloc] init];
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [edges addObjectsFromArray:[face edges]];
            id <Figure> figure = [self figureForFace:face];
            [geometryLayer removeFigure:figure];
            [selectionLayer addFigure:figure];
        }
        
        NSEnumerator* edgeEn = [edges objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject])) {
            id <Figure> figure = [self figureForEdge:edge];
            [geometryLayer removeFigure:figure];
            [selectionLayer addFigure:figure];
            [figure invalidate];
        }
        
        [edges release];
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
                id <Figure> figure = [self figureForFace:face];
                if (figure != nil) {
                    [selectionLayer removeFigure:figure];
                    [geometryLayer addFigure:figure];
                }
            }
            
            NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                id <Figure> figure = [self figureForEdge:edge];
                if (figure != nil) {
                    [selectionLayer removeFigure:figure];
                    [geometryLayer addFigure:figure];
                    [figure invalidate];
                }
            }
        }
    }
    
    if (faces != nil) {
        NSMutableSet* edges = [[NSMutableSet alloc] init];
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [edges addObjectsFromArray:[face edges]];
            id <Figure> figure = [self figureForFace:face];
            if (figure != nil) {
                [selectionLayer removeFigure:figure];
                [geometryLayer addFigure:figure];
            }
            
            NSEnumerator* edgeEn = [edges objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                id <Figure> figure = [self figureForEdge:edge];
                [selectionLayer removeFigure:figure];
                [geometryLayer addFigure:figure];
                [figure invalidate];
            }
            
            [edges release];
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
        [center addObserver:self selector:@selector(brushChanged:) name:BrushChanged object:map];
        [center addObserver:self selector:@selector(faceAdded:) name:FaceAdded object:map];
        [center addObserver:self selector:@selector(faceRemoved:) name:FaceRemoved object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceTextureChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged object:map];
        
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
