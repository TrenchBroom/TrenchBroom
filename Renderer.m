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
#import "MutableEntity.h"
#import "Brush.h"
#import "Face.h"
#import "Edge.h"
#import "Vertex.h"
#import "GeometryLayer.h"
#import "SelectionLayer.h"
#import "FigureLayer.h"
#import "DefaultEntityLayer.h"
#import "CompassFigure.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "SelectionManager.h"
#import "Brush.h"
#import "Face.h"
#import "MutableFace.h"
#import "Camera.h"
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
#import "DefaultFilter.h"
#import "EntityDefinition.h"

NSString* const RendererChanged = @"RendererChanged";

@interface Renderer (private)

- (void)validate;

- (void)addFace:(id <Face>)face;
- (void)removeFace:(id <Face>)face;
- (void)addBrush:(id <Brush>)brush;
- (void)removeBrush:(id <Brush>)brush;
- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)facesWillChange:(NSNotification *)notification;
- (void)facesDidChange:(NSNotification *)notification;
- (void)brushesWillChange:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)brushesAdded:(NSNotification *)notification;
- (void)brushesWillBeRemoved:(NSNotification *)notification;
- (void)entitiesAdded:(NSNotification *)notification;
- (void)entitiesWillBeRemoved:(NSNotification *)notification;
- (void)propertiesWillChange:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;

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
    TVector3f color;
    TVector2f gridCoords, texCoords;
    
    NSEnumerator* faceEn = [invalidFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        id <Brush> brush = [face brush];
        if (filter == nil || [filter isBrushRenderable:brush]) {
            NSArray* vertices = [face vertices];
            int vertexCount = [vertices count];
            
            VBOMemBlock* block = [face memBlock];
            if (block == nil || [block capacity] != vertexCount * vertexSize) {
                block = [sharedVbo allocMemBlock:vertexCount * vertexSize];
                [face setMemBlock:block];
            }
            
            color.x = [brush flatColor][0];
            color.y = [brush flatColor][1];
            color.z = [brush flatColor][2];
            
            Texture* texture = [textureManager textureForName:[face texture]];
            int width = texture != nil ? [texture width] : 1;
            int height = texture != nil ? [texture height] : 1;
            
            int offset = 0;
            NSEnumerator* vertexEn = [vertices objectEnumerator];
            Vertex* vertex;
            while ((vertex = [vertexEn nextObject])) {
                [face gridCoords:&gridCoords forVertex:[vertex vector]];
                [face texCoords:&texCoords forVertex:[vertex vector]];
                texCoords.x /= width;
                texCoords.y /= height;
                
                offset = [block writeVector2f:&gridCoords offset:offset];
                offset = [block writeVector2f:&texCoords offset:offset];
                offset = [block writeVector3f:&color offset:offset];
                offset = [block writeVector3f:[vertex vector] offset:offset];
            }
            
            [block setState:BS_USED_VALID];
        }
    }
    [invalidFaces removeAllObjects];
    
    [sharedVbo unmapBuffer];
    [sharedVbo deactivate];
}

- (void)addFace:(id <Face>)face {
    [invalidFaces addObject:face];
     
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isFaceSelected:face])
        [selectionLayer addFace:face];
    else 
        [geometryLayer addFace:face];
}

- (void)removeFace:(id <Face>)face {
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
    
    if (![entity isWorldspawn]) {
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isEntitySelected:entity])
            [selectionLayer addEntity:entity];
        else
            [entityLayer addEntity:entity];
    }
}

- (void)removeEntity:(id <Entity>)entity {
    NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];

    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager isEntitySelected:entity])
        [selectionLayer removeEntity:entity];
    else
        [entityLayer removeEntity:entity];
}

- (void)facesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:FacesKey];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isFaceSelected:face]) {
            [selectionLayer removeFace:face];
        } else {
            [geometryLayer removeFace:face];
        }
    }
}

- (void)facesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:FacesKey];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        [invalidFaces addObject:face];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        if ([selectionManager isFaceSelected:face]) {
            [selectionLayer addFace:face];
        } else {
            [geometryLayer addFace:face];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:BrushesKey];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> entity = [brush entity];
        
        if (![entity isWorldspawn])
            [self removeEntity:entity];
        else
            [self removeBrush:brush];
    }
}

- (void)brushesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:BrushesKey];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> entity = [brush entity];
        
        if (![entity isWorldspawn])
            [self addEntity:entity];
        else
            [self addBrush:brush];
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:BrushesKey];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self addBrush:brush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* brushes = [userInfo objectForKey:BrushesKey];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entitiesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:EntitiesKey];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [self addEntity:entity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entitiesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:EntitiesKey];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [self removeEntity:entity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)propertiesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:EntitiesKey];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [self removeEntity:entity];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:EntitiesKey];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [self addEntity:entity];

    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (entities != nil) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [entityLayer removeEntity:entity];
            [selectionLayer addEntity:entity];
        }
    }
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [geometryLayer removeBrush:brush];
            [selectionLayer addBrush:brush];
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [geometryLayer removeFace:face];
            [selectionLayer addFace:face];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* entities = [userInfo objectForKey:SelectionEntities];
    NSSet* brushes = [userInfo objectForKey:SelectionBrushes];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    if (entities != nil) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [selectionLayer removeEntity:entity];
            [entityLayer addEntity:entity];
        }
    }
    
    if (brushes != nil) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            [selectionLayer removeBrush:brush];
            [geometryLayer addBrush:brush];
        }
    }
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [selectionLayer removeFace:face];
            [geometryLayer addFace:face];
        }
    }
    
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

    [filter release];
    filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager options:options];
    
    [geometryLayer setFilter:filter];
    [selectionLayer setFilter:filter];
    [entityLayer setFilter:filter];

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
    if ((self = [self init])) {
        windowController = theWindowController;

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
        entityLayer = [[DefaultEntityLayer alloc] initWithOptions:options];
        selectionLayer = [[SelectionLayer alloc] initWithVbo:sharedVbo textureManager:textureManager options:options camera:camera fontManager:fontManager font:trackingFont];
        feedbackLayer = [[FigureLayer alloc] init];

        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager options:options];
        [geometryLayer setFilter:filter];
        [selectionLayer setFilter:filter];
        [entityLayer setFilter:filter];
        
        compassFigure = [[CompassFigure alloc] initWithCamera:camera];
        
        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        
        [center addObserver:self selector:@selector(entitiesAdded:) name:EntitiesAdded object:map];
        [center addObserver:self selector:@selector(entitiesWillBeRemoved:) name:EntitiesWillBeRemoved object:map];
        [center addObserver:self selector:@selector(propertiesWillChange:) name:PropertiesWillChange object:map];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:map];
        [center addObserver:self selector:@selector(brushesAdded:) name:BrushesAdded object:map];
        [center addObserver:self selector:@selector(brushesWillBeRemoved:) name:BrushesWillBeRemoved object:map];
        [center addObserver:self selector:@selector(brushesWillChange:) name:BrushesWillChange object:map];
        [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:map];
        [center addObserver:self selector:@selector(facesWillChange:) name:FacesWillChange object:map];
        [center addObserver:self selector:@selector(facesDidChange:) name:FacesDidChange object:map];
        
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
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

- (void)render {
    [self validate];
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    
    [geometryLayer render];
    [entityLayer render];
    [selectionLayer render];
    [feedbackLayer render];
    
    // enable lighting for cursor and compass
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    
    Camera* camera = [windowController camera];
    
    glEnable(GL_LIGHT0);
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { [camera position]->x, [camera position]->y, [camera position]->z, 1.0f };
    
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

    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 0.4f);
    [cursorManager render];

    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 1);
    [cursorManager render];
    
    glClear(GL_DEPTH_BUFFER_BIT);
    [compassFigure render];

    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    
    // brightness
    /*
    float brightness = 0.5f;
    if( brightness > 1 )
    {
        glBlendFunc( GL_DST_COLOR, GL_ONE );
        glColor3f( brightness-1, brightness-1, brightness-1 );
    }
    else
    {
        glBlendFunc( GL_ZERO, GL_SRC_COLOR );
        glColor3f( brightness, brightness, brightness );
    }
    glEnable( GL_BLEND );
     */
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [compassFigure release];
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
