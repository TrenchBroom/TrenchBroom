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
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"

NSString* const RendererChanged = @"RendererChanged";

@implementation Renderer

- (FaceFigure *)createFaceFigure:(Face *)theFace {
    MapDocument* document = [windowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* vbo = [glResources geometryVBO];
    
    FaceFigure* faceFigure = [[FaceFigure alloc] initWithFace:theFace vbo:vbo];
    [faceFigures setObject:faceFigure forKey:[theFace faceId]];
    return [faceFigure autorelease];
}

- (void)addFace:(Face *)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face]) {
        [selectionLayer addFigure:[self createFaceFigure:face]];
    } else {
        [geometryLayer addFigure:[self createFaceFigure:face]];
    }
    
    [face addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged];
    [face addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged];
}

- (void)removeFace:(Face *)face {
    SelectionManager* selectionManager = [windowController selectionManager];
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

- (void)cameraChanged:(NSNotification *)notification {
    [self notifyObservers:RendererChanged];
}

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];

        MapDocument* mapDocument = [windowController document];
        GLResources* glResources = [mapDocument glResources];
        VBOBuffer* vbo = [glResources geometryVBO];
        
        geometryLayer = [[GeometryLayer alloc] initWithVbo:vbo];
        selectionLayer = [[SelectionLayer alloc] initWithVbo:vbo];
        toolLayer = [[ToolLayer alloc] init];

        Map* map = [mapDocument map];
        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        Entity* entity;
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];

        [map addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded];
        [map addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        [selectionManager addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded];
        [selectionManager addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved];
        
        Camera* camera = [windowController camera];
        [camera addObserver:self selector:@selector(cameraChanged:) name:CameraChanged];
    }
    
    return self;
}

- (void)render {
    MapDocument* document = [windowController document];
    GLResources* glResources = [document glResources];
    TextureManager* textureManager = [glResources textureManager];
    Options* options = [windowController options];
    
    RenderContext* renderContext = [[RenderContext alloc] initWithTextureManager:textureManager options:options];
    [geometryLayer render:renderContext];
    [selectionLayer render:renderContext];
    [toolLayer render:renderContext];
}

- (void)updateView:(NSRect)bounds {
    Camera* camera = [windowController camera];
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
    Map* map = [[windowController document] map];
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
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager removeObserver:self];

    Camera* camera = [windowController camera];
    [camera removeObserver:self];
    
    [windowController release];
    
    [geometryLayer release];
    [selectionLayer release];
    [toolLayer release];
    [faceFigures release];
    [super dealloc];
}

@end
