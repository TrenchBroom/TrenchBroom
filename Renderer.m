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
#import "ToolLayer.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "RenderContext.h"
#import "SelectionManager.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "Vector3f.h"
#import "FaceFigure.h"
#import "Options.h"
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"

NSString* const RendererChanged = @"RendererChanged";

@implementation Renderer

- (FaceFigure *)figureForFace:(id <Face>)face create:(BOOL)create {
    FaceFigure* figure = [faceFigures objectForKey:[face faceId]];
    if (figure == nil && create) {
        MapDocument* mapDocument = [windowController document];
        GLResources* glResources = [mapDocument glResources];
        VBOBuffer* vbo = [glResources geometryVBO];
        
        figure = [[FaceFigure alloc] initWithFace:face vbo:vbo];
        [faceFigures setObject:figure forKey:[face faceId]];
        [figure release];
    }
    
    return figure;
}

- (void)addFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    FaceFigure* figure = [self figureForFace:face create:YES];
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer addFigure:figure];
    else 
        [geometryLayer addFigure:figure];
}

- (void)removeFace:(id <Face>)face {
    SelectionManager* selectionManager = [windowController selectionManager];
    FaceFigure* figure = [self figureForFace:face create:NO];
    if (figure == nil)
        [NSException raise:NSInvalidArgumentException format:@"face %@ has no figure", face];
    
    if ([selectionManager isFaceSelected:face] || [selectionManager isBrushSelected:[face brush]])
        [selectionLayer removeFigure:figure];
    else
        [geometryLayer removeFigure:figure];
    
    [faceFigures removeObjectForKey:[face faceId]];
}

- (void)addBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];
}

- (void)removeBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
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
    FaceFigure* figure = [self figureForFace:face create:NO];
    if (figure != nil) {
        [figure invalidate];
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
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                FaceFigure* figure = [self figureForFace:face create:NO];
                [geometryLayer removeFigure:figure];
                [selectionLayer addFigure:figure];
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            FaceFigure* figure = [self figureForFace:face create:NO];
            [geometryLayer removeFigure:figure];
            [selectionLayer addFigure:figure];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                FaceFigure* figure = [self figureForFace:face create:NO];
                if (figure != nil) {
                    [selectionLayer removeFigure:figure];
                    [geometryLayer addFigure:figure];
                }
            }
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            FaceFigure* figure = [self figureForFace:face create:NO];
            if (figure != nil) {
                [selectionLayer removeFigure:figure];
                [geometryLayer addFigure:figure];
            }
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
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

        geometryLayer = [[GeometryLayer alloc] initWithWindowController:windowController];
        selectionLayer = [[SelectionLayer alloc] initWithWindowController:windowController];
        toolLayer = [[ToolLayer alloc] init];

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
        [center addObserver:self selector:@selector(faceAdded:) name:FaceAdded object:map];
        [center addObserver:self selector:@selector(faceRemoved:) name:FaceRemoved object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceTextureChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged object:map];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
        Camera* camera = [windowController camera];
        [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
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
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [windowController release];
    [geometryLayer release];
    [selectionLayer release];
    [toolLayer release];
    [faceFigures release];
    [super dealloc];
}

@end
