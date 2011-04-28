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
#import "FigureLayer.h"
#import "EntityLayer.h"
#import "CompassFigure.h"
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
#import "Vector2f.h"
#import "Options.h"
#import "Grid.h"
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "GLFontManager.h"
#import "Texture.h"
#import "CompassFigure.h"
#import "CursorManager.h"
#import "Figure.h"
#import "Filter.h"
#import "SelectionFilter.h"

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
- (void)brushWillBeRemoved:(NSNotification *)notification;
- (void)entityAdded:(NSNotification *)notification;
- (void)entityWillBeRemoved:(NSNotification *)notification;

- (void)selectionAdded:(NSNotification *)notification;
- (void)selectionRemoved:(NSNotification *)notification;
- (void)trackedObjectChanged:(NSNotification *)notification;
- (void)textureManagerChanged:(NSNotification *)notification;
- (void)cameraChanged:(NSNotification *)notification;
- (void)optionsChanged:(NSNotification *)notification;
- (void)gridChanged:(NSNotification *)notification;
- (void)cursorChanged:(NSNotification *)notification;

@end

@implementation Renderer (private)

- (void)validate {
    if ([invalidFaces count] == 0)
        return;
    
    [sharedVbo activate];
    [sharedVbo mapBuffer];
    
    int vertexSize = 10 * sizeof(float);
    Vector3f* color = [[Vector3f alloc] init];
    Vector2f* gridCoords = [[Vector3f alloc] init];
    Vector2f* texCoords = [[Vector2f alloc] init];
    
    NSEnumerator* faceEn = [invalidFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        id <Brush> brush = [face brush];
        if (filter == nil || [filter brushPasses:brush]) {
            NSArray* vertices = [face vertices];
            int vertexCount = [vertices count];
            
            VBOMemBlock* block = [face memBlock];
            if (block == nil || [block capacity] != vertexCount * vertexSize) {
                block = [sharedVbo allocMemBlock:vertexCount * vertexSize];
                [face setMemBlock:block];
            }
            
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
                [face gridCoords:gridCoords forVertex:[vertex vector]];
                [face texCoords:texCoords forVertex:[vertex vector]];
                [texCoords setX:[texCoords x] / width];
                [texCoords setY:[texCoords y] / height];
                
                offset = [block writeVector2f:gridCoords offset:offset];
                offset = [block writeVector2f:texCoords offset:offset];
                offset = [block writeVector3f:color offset:offset];
                offset = [block writeVector3f:[vertex vector] offset:offset];
            }
            
            [block setState:BS_USED_VALID];
        }
    }
    [color release];
    [gridCoords release];
    [texCoords release];
    [invalidFaces removeAllObjects];
    
    [sharedVbo unmapBuffer];
    [sharedVbo deactivate];
}

- (void)addFace:(id <Face>)face {
    [invalidFaces addObject:face];

    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer addFace:face];
    */
     
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face])
        [selectionLayer addFace:face];
    else 
        [geometryLayer addFace:face];
}

- (void)removeFace:(id <Face>)face {
    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer removeFace:face];
     */
    
    SelectionManager* selectionManager = [windowController selectionManager];
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

    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isBrushTracked:brush])
        [trackingLayer addBrush:brush];
    */
}

- (void)removeBrush:(id <Brush>)brush {
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
    
    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isBrushTracked:brush])
        [trackingLayer removeBrush:brush];
     */
}

- (void)addEntity:(id <Entity>)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self addBrush:brush];
    
    [entityLayer addEntity:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];

    [entityLayer removeEntity:entity];
}

- (void)faceWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Face> face = [userInfo objectForKey:FaceKey];

    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer removeFace:face];
    */
     
    SelectionManager* selectionManager = [windowController selectionManager];
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

    /*
    TrackingManager* trackingManager = [windowController trackingManager];
    if ([trackingManager isFaceTracked:face])
        [trackingLayer addFace:face];
    */
    
    SelectionManager* selectionManager = [windowController selectionManager];
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

- (void)brushWillBeRemoved:(NSNotification *)notification {
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

- (void)entityWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    [self removeEntity:entity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
//    TrackingManager* trackingManager = [windowController trackingManager];
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [geometryLayer removeBrush:brush];
            [selectionLayer addBrush:brush];

            /*
            if ([trackingManager isBrushTracked:brush])
                [trackingLayer addBrush:brush];
             */
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [geometryLayer removeFace:face];
            [selectionLayer addFace:face];
            
            /*
            if ([trackingManager isFaceTracked:face])
                [trackingLayer addFace:face];
             */
        }
    }
    
    Options* options = [windowController options];
    if ([options isolationMode] != IM_DISCARD)
        [self setFilter:nil];
    else
        [self setFilter:[[[SelectionFilter alloc] initWithSelectionManager:[windowController selectionManager]] autorelease]];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    //TrackingManager* trackingManager = [windowController trackingManager];
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [selectionLayer removeBrush:brush];
            [geometryLayer addBrush:brush];

            /*
            if ([trackingManager isBrushTracked:brush])
                [trackingLayer removeBrush:brush];
             */
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [selectionLayer removeFace:face];
            [geometryLayer addFace:face];
            
            /*
            if ([trackingManager isFaceTracked:face])
                [trackingLayer removeFace:face];
             */
        }
    }
    
    Options* options = [windowController options];
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([options isolationMode] != IM_DISCARD || [selectionManager mode] == SM_UNDEFINED)
        [self setFilter:nil];
    else
        [self setFilter:[[[SelectionFilter alloc] initWithSelectionManager:selectionManager] autorelease]];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)trackedObjectChanged:(NSNotification *)notification {
    /*
    NSDictionary* userInfo = [notification userInfo];
    id untrackedObject = [userInfo objectForKey:UntrackedObjectKey];
    id trackedObject = [userInfo objectForKey:TrackedObjectKey];
    
    if (untrackedObject != nil) {
        if ([untrackedObject conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)untrackedObject;
            [trackingLayer removeBrush:brush];
        }
    }

    if (trackedObject != nil) {
        if ([trackedObject conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)trackedObject;
            [trackingLayer addBrush:brush];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
     */
}

- (void)textureManagerChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)optionsChanged:(NSNotification *)notification {
    SelectionManager* selectionManager = [windowController selectionManager];
    Options* options = [windowController options];
    if ([options isolationMode] != IM_DISCARD || [selectionManager mode] == SM_UNDEFINED)
        [self setFilter:nil];
    else
        [self setFilter:[[[SelectionFilter alloc] initWithSelectionManager:selectionManager] autorelease]];

    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)gridChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cursorChanged:(NSNotification *)notification {
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
        GLFontManager* fontManager = [glResources fontManager];
        NSFont* trackingFont = [NSFont systemFontOfSize:11];
        textureManager = [[glResources textureManager] retain];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        Camera* camera = [windowController camera];
        Options* options = [windowController options];
        Grid* grid = [options grid];

        geometryLayer = [[GeometryLayer alloc] initWithVbo:sharedVbo textureManager:textureManager options:options];
        entityLayer = [[EntityLayer alloc] initWithEntityDefinitionManager:[map entityDefinitionManager]];
        selectionLayer = [[SelectionLayer alloc] initWithVbo:sharedVbo textureManager:textureManager options:options camera:camera fontManager:fontManager font:trackingFont];
        feedbackLayer = [[FigureLayer alloc] init];

        compassFigure = [[CompassFigure alloc] initWithCamera:camera];
        
        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        
        [center addObserver:self selector:@selector(entityAdded:) name:EntityAdded object:map];
        [center addObserver:self selector:@selector(entityWillBeRemoved:) name:EntityWillBeRemoved object:map];
        [center addObserver:self selector:@selector(brushAdded:) name:BrushAdded object:map];
        [center addObserver:self selector:@selector(brushWillBeRemoved:) name:BrushWillBeRemoved object:map];
        [center addObserver:self selector:@selector(brushWillChange:) name:BrushWillChange object:map];
        [center addObserver:self selector:@selector(brushDidChange:) name:BrushDidChange object:map];
        [center addObserver:self selector:@selector(faceWillChange:) name:FaceWillChange object:map];
        [center addObserver:self selector:@selector(faceDidChange:) name:FaceDidChange object:map];
        
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
        TrackingManager* trackingManager = [windowController trackingManager];
        [center addObserver:self selector:@selector(trackedObjectChanged:) name:TrackedObjectChanged object:trackingManager];

        [center addObserver:self selector:@selector(textureManagerChanged:) name:TextureManagerChanged object:textureManager];
        [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
        [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
        [center addObserver:self selector:@selector(gridChanged:) name:GridChanged object:grid];
        
        CursorManager* cursorManager = [windowController cursorManager];
        [center addObserver:self selector:@selector(cursorChanged:) name:CursorChanged object:cursorManager];
    }
    
    return self;
}

- (void)addFeedbackFigure:(id <Figure>)theFigure {
    [feedbackLayer addFigure:theFigure];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)removeFeedbackFigure:(id <Figure>)theFigure {
    [feedbackLayer removeFigure:theFigure];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)setFilter:(id <Filter>)theFilter {
    if (filter == theFilter)
        return;
    
    [filter release];
    filter = [theFilter retain];
    
    [geometryLayer setFilter:filter];
    [selectionLayer setFilter:filter];
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
    [entityLayer render:renderContext];
    [selectionLayer render:renderContext];
    [feedbackLayer render:renderContext];
    
    // enable lighting for cursor and compass
    glEnable(GL_LIGHTING);
    
    Camera* camera = [windowController camera];
    
    glEnable(GL_LIGHT0);
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { [[camera position] x], [[camera position] y], [[camera position] z], 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
    glMateriali(GL_FRONT, GL_SHININESS, 96);
    
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager render];

    glClear(GL_DEPTH_BUFFER_BIT);
    [compassFigure render];

    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);

    [renderContext release];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [compassFigure release];
    [windowController release];
    [geometryLayer release];
    [selectionLayer release];
    [feedbackLayer release];
    [entityLayer release];
    [textureManager release];
    [sharedVbo release];
    [invalidFaces release];
    [filter release];
    [super dealloc];
}

@end
