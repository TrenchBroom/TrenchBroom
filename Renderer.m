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
#import "Edge.h"
#import "Vertex.h"
#import "GeometryLayer.h"
#import "SelectionLayer.h"
#import "TrackingLayer.h"
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
#import "Grid.h"
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "Texture.h"

NSString* const RendererChanged = @"RendererChanged";

@interface Renderer (private)

- (void)validate;

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
- (void)gridChanged:(NSNotification *)notification;

@end

@implementation Renderer (private)

- (void)validate {
    if ([invalidFaces count] == 0)
        return;
    
    [sharedVbo activate];
    [sharedVbo mapBuffer];
    
    Vector3f* color = [[Vector3f alloc] init];
    Vector2f* texCoords = [[Vector2f alloc] init];
    
    NSEnumerator* faceEn = [invalidFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        NSArray* vertices = [face vertices];
        
        int vertexSize = 8 * sizeof(float);
        int vertexCount = [vertices count];
        if (vertexCount == 0)
            NSLog(@"asdf");
        
        VBOMemBlock* block = [face memBlock];
        if (block == nil || [block capacity] != vertexCount * vertexSize) {
            block = [sharedVbo allocMemBlock:vertexCount * vertexSize];
            [face setMemBlock:block];
        }
        
        id <Brush> brush = [face brush];
        [color setX:[brush flatColor][0]];
        [color setY:[brush flatColor][1]];
        [color setZ:[brush flatColor][2]];
        
        Texture* texture = [textureManager textureForName:[face texture]];
        int width = texture != nil ? [texture width] : 1;
        int height = texture != nil ? [texture height] : 1;
        
        int offset = 0;
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex;
        while ((vertex = [vertexEn nextObject])) {
            [face texCoords:texCoords forVertex:[vertex vector]];
            [texCoords setX:[texCoords x] / width];
            [texCoords setY:[texCoords y] / height];
            offset = [block writeVector2f:texCoords offset:offset];
            offset = [block writeVector3f:color offset:offset];
            offset = [block writeVector3f:[vertex vector] offset:offset];
        }
        
        [block setState:BS_USED_VALID];
    }
    [color release];
    [texCoords release];
    [invalidFaces removeAllObjects];
    
    [sharedVbo unmapBuffer];
    [sharedVbo deactivate];
}

- (void)addFace:(id <Face>)face {
    [invalidFaces addObject:face];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer addFace:face];
    
    if ([selectionManager isFaceSelected:face])
        [selectionLayer addFace:face];
    else 
        [geometryLayer addFace:face];
}

- (void)removeFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer removeFace:face];
    
    if ([selectionManager isFaceSelected:face])
        [selectionLayer removeFace:face];
    else
        [geometryLayer removeFace:face];

    [face setMemBlock:nil];
}

- (void)addBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];

    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isBrushTracked:brush])
        [trackingLayer addBrush:brush];
    
    /*
    SelectionManager* selectionManager = [windowController selectionManager];
    TrackingManager* trackingManager = [windowController trackingManager];

    NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject])) {
        if ([trackingManager isEdgeTracked:edge])
            [feedbackLayer addEdge:edge];
        else if ([selectionManager isEdgeSelected:edge])
            [selectionLayer addEdge:edge];
        else
            [geometryLayer addEdge:edge];
    }
    
    NSEnumerator* vertexEn = [[brush vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject])) {
        if ([trackingManager isVertexTracked:vertex])
            [feedbackLayer addVertex:vertex];
        else if ([selectionManager isVertexSelected:vertex])
            [selectionLayer addVertex:vertex];
    }
     */
}

- (void)removeBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
    
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isBrushTracked:brush])
        [trackingLayer removeBrush:brush];
    
    /*
    SelectionManager* selectionManager = [windowController selectionManager];
    TrackingManager* trackingManager = [windowController trackingManager];

    NSEnumerator* edgeEn = [[brush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject])) {
        if ([trackingManager isEdgeTracked:edge])
            [feedbackLayer removeEdge:edge];
        else if ([selectionManager isEdgeSelected:edge])
            [selectionLayer removeEdge:edge];
        else
            [geometryLayer removeEdge:edge];
    }
    
    NSEnumerator* vertexEn = [[brush vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject])) {
        if ([trackingManager isVertexTracked:vertex])
            [feedbackLayer removeVertex:vertex];
        else if ([selectionManager isVertexSelected:vertex])
            [selectionLayer removeVertex:vertex];
    }
     */
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
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer removeFace:face];
    
    if ([selectionManager isFaceSelected:face]) {
        [selectionLayer removeFace:face];
    } else {
        [geometryLayer removeFace:face];
    }
}

- (void)faceDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];

    [invalidFaces addObject:face];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer addFace:face];
    
    if ([selectionManager isFaceSelected:face]) {
        [selectionLayer addFace:face];
    } else {
        [geometryLayer addFace:face];
    }
    
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
    
    TrackingManager* trackingManager = [windowController trackingManager];
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [geometryLayer removeBrush:brush];
            [selectionLayer addBrush:brush];

            if ([trackingManager isBrushTracked:brush])
                [trackingLayer addBrush:brush];
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [geometryLayer removeFace:face];
            [selectionLayer addFace:face];
            
            if ([trackingManager isFaceTracked:face])
                [trackingLayer addFace:face];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    TrackingManager* trackingManager = [windowController trackingManager];
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [selectionLayer removeBrush:brush];
            [geometryLayer addBrush:brush];

            if ([trackingManager isBrushTracked:brush])
                [trackingLayer removeBrush:brush];
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [selectionLayer removeFace:face];
            [geometryLayer addFace:face];
            
            if ([trackingManager isFaceTracked:face])
                [trackingLayer removeFace:face];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)trackedObjectChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id untrackedObject = [userInfo objectForKey:UntrackedObjectKey];
    id trackedObject = [userInfo objectForKey:TrackedObjectKey];
    
    if (untrackedObject != nil) {
        if ([untrackedObject isKindOfClass:[Vertex class]]) {
            Vertex* vertex = (Vertex *)untrackedObject;
            [trackingLayer removeVertex:vertex];
        } else if ([untrackedObject isKindOfClass:[Edge class]]) {
            Edge* edge = (Edge *)untrackedObject;
            [trackingLayer removeEdge:edge];
        } else if ([untrackedObject conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)untrackedObject;
            [trackingLayer removeBrush:brush];
        }
    }

    if (trackedObject != nil) {
        if ([trackedObject isKindOfClass:[Vertex class]]) {
            Vertex* vertex = (Vertex *)trackedObject;
            [trackingLayer addVertex:vertex];
        } else if ([trackedObject isKindOfClass:[Edge class]]) {
            Edge* edge = (Edge *)trackedObject;
            [trackingLayer addEdge:edge];
        } else if ([trackedObject conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)trackedObject;
            [trackingLayer addBrush:brush];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)optionsChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)gridChanged:(NSNotification *)notification {
    Grid* grid = [notification object];
    [selectionLayer setGridSize:[grid size]];
    [selectionLayer setDrawGrid:[grid draw]];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

@end

@implementation Renderer

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];

        sharedVbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        invalidFaces = [[NSMutableSet alloc] init];
        
        MapDocument* map = [windowController document];
        GLResources* glResources = [map glResources];
        textureManager = [[glResources textureManager] retain];
        
        Options* options = [windowController options];
        Grid* grid = [options grid];

        geometryLayer = [[GeometryLayer alloc] initWithVbo:sharedVbo textureManager:textureManager];
        selectionLayer = [[SelectionLayer alloc] initWithVbo:sharedVbo textureManager:textureManager gridSize:[grid size] drawGrid:[grid draw]];
        trackingLayer = [[TrackingLayer alloc] init];

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
        
        [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
        [center addObserver:self selector:@selector(gridChanged:) name:GridChanged object:grid];
    }
    
    return self;
}

- (void)render {
    [self validate];
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    
    Options* options = [windowController options];
    
    RenderContext* renderContext = [[RenderContext alloc] initWithOptions:options];
    [geometryLayer render:renderContext];
    [selectionLayer render:renderContext];
    [trackingLayer render:renderContext];
    
    [renderContext release];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [windowController release];
    [geometryLayer release];
    [selectionLayer release];
    [trackingLayer release];
    [textureManager release];
    [sharedVbo release];
    [invalidFaces release];
    [super dealloc];
}

@end
