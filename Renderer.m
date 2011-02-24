//
//  Renderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Renderer.h"
#import <OpenGL/glu.h>
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "GeometryLayer.h"
#import "SelectionLayer.h"
#import "ToolLayer.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "RenderContext.h"
#import "SelectionManager.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "Vector3f.h"
#import "ToolManager.h"
#import "FaceFigure.h"
#import "Options.h"

NSString* const RendererChanged = @"RendererChanged";

@implementation Renderer

- (FaceFigure *)createFaceFigure:(Face *)theFace {
    FaceFigure* faceFigure = [[FaceFigure alloc] initWithFace:theFace vbo:vbo];
    [faceFigures setObject:faceFigure forKey:[theFace faceId]];
    return [faceFigure autorelease];
}

- (void)addFace:(Face *)face {
    if ([selectionManager isFaceSelected:face]) {
        [selectionLayer addFigure:[self createFaceFigure:face]];
    } else {
        [geometryLayer addFigure:[self createFaceFigure:face]];
    }
    
    [face addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged];
    [face addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged];
}

- (void)removeFace:(Face *)face {
    FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
    if ([selectionManager isFaceSelected:face])
        [selectionLayer removeFigure:faceFigure];
    else
        [geometryLayer removeFigure:faceFigure];
    
    [faceFigures removeObjectForKey:[face faceId]];
    [face removeObserver:self];
}

- (void)addBrush:(Brush *)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];

    [brush addObserver:self selector:@selector(faceAdded:) name:BrushFaceAdded];
    [brush addObserver:self selector:@selector(faceRemoved:) name:BrushFaceRemoved];
}

- (void)removeBrush:(Brush *)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];

    [brush removeObserver:self];
}

- (void)addEntity:(Entity *)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    Brush* brush;
    while ((brush = [brushEn nextObject]))
        [self addBrush:brush];
    
    [entity addObserver:self selector:@selector(brushAdded:) name:EntityBrushAdded];
    [entity addObserver:self selector:@selector(brushAdded:) name:EntityBrushRemoved];
}

- (void)removeEntity:(Entity *)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    Brush* brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];

    [entity removeObserver:self];
}

- (void)faceChanged:(NSNotification *)notification {
    [self notifyObservers:RendererChanged];
}

- (void)faceAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Face* face = [userInfo objectForKey:FaceKey];
    [self addFace:face];
    
    [self notifyObservers:RendererChanged];
}

- (void)faceRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Face* face = [userInfo objectForKey:FaceKey];
    [self removeFace:face];
    
    [self notifyObservers:RendererChanged];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:BrushKey];
    [self addBrush:brush];
    
    [self notifyObservers:RendererChanged];
}

- (void)brushRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:BrushKey];
    [self removeBrush:brush];
    
    [self notifyObservers:RendererChanged];
}

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:EntityKey];
    [self addEntity:entity];
    
    [self notifyObservers:RendererChanged];
}

- (void)entityRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:EntityKey];
    [self removeEntity:entity];
    
    [self notifyObservers:RendererChanged];
}

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithMap:(Map *)theMap vbo:(VBOBuffer *)theVbo {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    if (theVbo == nil)
        [NSException raise:NSInvalidArgumentException format:@"vbo must not be nil"];
    
    if (self = [self init]) {
        map =[theMap retain];
        vbo = [theVbo retain];
        geometryLayer = [[GeometryLayer alloc] initWithVbo:vbo];
        selectionLayer = [[SelectionLayer alloc] initWithVbo:vbo];
        toolLayer = [[ToolLayer alloc] init];

        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        Entity* entity;
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];

        [map addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded];
        [map addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved];
    }
    
    return self;
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            Face* face;
            while ((face = [faceEn nextObject])) {
                FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
                [geometryLayer removeFigure:faceFigure];
                [selectionLayer addFigure:faceFigure];
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
            FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
            [geometryLayer removeFigure:faceFigure];
            [selectionLayer addFigure:faceFigure];
        }
    }

    [self notifyObservers:RendererChanged];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            Face* face;
            while ((face = [faceEn nextObject])) {
                FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
                [selectionLayer removeFigure:faceFigure];
                [geometryLayer addFigure:faceFigure];
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
            FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
            [selectionLayer removeFigure:faceFigure];
            [geometryLayer addFigure:faceFigure];
        }
    }

    [self notifyObservers:RendererChanged];
}

- (void)setSelectionManager:(SelectionManager *)theSelectionManager {
    if (theSelectionManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"selection manager must not be nil"];
    
    if (selectionManager != nil) {
        [selectionManager removeObserver:self];
        [selectionManager release];
    }
    
    selectionManager = [theSelectionManager retain];
    [selectionManager addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded];
    [selectionManager addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved];
}

- (void)toolFiguresAdded:(NSNotification *)notification {
    NSDictionary* userInfo= [notification userInfo];
    NSSet* figures = [userInfo objectForKey:FiguresKey];

    NSEnumerator* figureEn = [figures objectEnumerator];
    id figure;
    while ((figure = [figureEn nextObject]))
        [toolLayer addFigure:figure];
}

- (void)toolFiguresRemoved:(NSNotification *)notification {
    NSDictionary* userInfo= [notification userInfo];
    NSSet* figures = [userInfo objectForKey:FiguresKey];
    
    NSEnumerator* figureEn = [figures objectEnumerator];
    id figure;
    while ((figure = [figureEn nextObject]))
        [toolLayer removeFigure:figure];
}

- (void)setToolManager:(ToolManager *)theToolManager {
    if (theToolManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"tool manager must not be nil"];
    
    if (toolManager != nil) {
        [toolManager removeObserver:self];
        [toolManager release];
    }
    
    toolManager = [theToolManager retain];
    [toolManager addObserver:self selector:@selector(toolFiguresAdded:) name:FiguresAdded];
    [toolManager addObserver:self selector:@selector(toolFiguresRemoved:) name:FiguresRemoved];
}

- (void)cameraChanged:(NSNotification *)notification {
    [self notifyObservers:RendererChanged];
}

- (void)setCamera:(Camera *)theCamera {
    if (theCamera == nil)
        [NSException raise:NSInvalidArgumentException format:@"camera must not be nil"];
    
    if (camera != nil) {
        [camera removeObserver:self];
        [camera release];
    }
    
    camera = [theCamera retain];
    [camera addObserver:self selector:@selector(cameraChanged:) name:CameraChanged];
}

- (void)render:(RenderContext *)renderContext {
    [geometryLayer render:renderContext];
    [selectionLayer render:renderContext];
    [toolLayer render:renderContext];
}

- (void)updateView:(NSRect)bounds {
    // float fov = [camera fieldOfVision];
    float fov = atan(tan(90 * M_PI / 360) * 0.75 * bounds.size.width / bounds.size.height) * 360 / M_PI;
    float aspect = bounds.size.width / bounds.size.height;
    float near = [camera nearClippingPlane];
    float far = [camera farClippingPlane];
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, near, far);
    
    Vector3f* pos = [camera position];
    Vector3f* dir = [camera direction];
    Vector3f* up = [camera up];
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt([pos x],
              [pos y],
              [pos z],
              [pos x] + [dir x],
              [pos y] + [dir y],
              [pos z] + [dir z],
              [up x],
              [up y],
              [up z]);
}

- (void)dealloc {
    NSEnumerator* entityEn = [[map entities] objectEnumerator];
    Entity* entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            Face* face;
            while ((face = [faceEn nextObject])) {
                [face removeObserver:self];
            }
            [brush removeObserver:self];
        }
        [entity removeObserver:self];
    }
    [map removeObserver:self];
    
    [selectionManager removeObserver:self];
    [toolManager removeObserver:self];
    [camera removeObserver:self];
    
    [geometryLayer release];
    [selectionLayer release];
    [toolLayer release];
    [faceFigures release];
    [vbo release];
    [toolManager release];
    [selectionManager release];
    [camera release];
    [map release];
    [super dealloc];
}

@end
