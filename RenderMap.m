//
//  RenderMap.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderMap.h"
#import <OpenGL/glu.h>
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "RenderEntity.h"
#import "RenderBrush.h"
#import "VBOBuffer.h"
#import "Camera.h"
#import "TextureManager.h"
#import "SelectionManager.h"
#import "IntData.h"
#import "Vector3f.h"

@implementation RenderMap

- (id)init {
    if (self = [super init]) {
        map = nil;
        renderEntities = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addEntity:(Entity *)theEntity {
    RenderEntity* renderEntity = [[RenderEntity alloc] initInMap:self withEntity:theEntity faceVBO:faceVBO];
    [renderEntities setObject:renderEntity forKey:[theEntity entityId]];
    [renderEntity release];
}

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];
    [self addEntity:entity];
}

- (void)entityRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];
    [renderEntities removeObjectForKey:[entity entityId]];
}

- (void)selectionChanged:(NSNotification *)notification {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    [selIndexBuffers removeAllObjects];
    [selCountBuffers removeAllObjects];
    buffersValid = NO;
}

- (id)initWithMap:(Map *)theMap faceVBO:(VBOBuffer *)theFaceVBO camera:(Camera *)theCamera textureManager:(TextureManager *)theTextureManager selectionManager:(SelectionManager *)theSelectionManager {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];
    if (theCamera == nil)
        [NSException raise:NSInvalidArgumentException format:@"camera must not be nil"];
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    if (theSelectionManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"selection manager must not be nil"];

    if (self = [self init]) {
        map = [theMap retain];
        faceVBO = [theFaceVBO retain];
        camera = [theCamera retain];
        textureManager = [theTextureManager retain];
        selectionManager = [theSelectionManager retain];
        
        // map texture names to index and count buffers for efficient texture rendering
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
        
        // same for selection
        selIndexBuffers = [[NSMutableDictionary alloc] init];
        selCountBuffers = [[NSMutableDictionary alloc] init];
        buffersValid = NO;

        NSArray* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded object:map];
        [center addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved object:map];
        [center addObserver:self selector:@selector(selectionChanged:) name:SelectionChanged object:selectionManager];
    }
    
    return self;
}

- (NSArray *)renderEntities {
    return [renderEntities allValues];
}

- (void)entityChanged {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    [selIndexBuffers removeAllObjects];
    [selCountBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)preRender {
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    [faceVBO activate];
}

- (void)prepareFaces {
    [faceVBO mapBuffer];
    
    NSEnumerator* renderEntityEn = [renderEntities objectEnumerator];
    RenderEntity* renderEntity;
    while ((renderEntity = [renderEntityEn nextObject])) {
        NSEnumerator* renderBrushEn = [[renderEntity renderBrushes] objectEnumerator];
        RenderBrush* renderBrush;
        while ((renderBrush = [renderBrushEn nextObject]))
            [renderBrush prepareFacesWithTextureManager:textureManager];
    }
    
    [faceVBO unmapBuffer];
}

- (void)renderTexturedPolygonsWithIndexBuffers:(NSMutableDictionary *)theIndexBuffers countBuffers:(NSMutableDictionary *)theCountBuffers {
    NSEnumerator* textureNameEn = [theIndexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        Texture* texture = [textureManager textureForName:textureName];
        [texture activate];
        
        IntData* indexBuffer = [theIndexBuffers objectForKey:textureName];
        IntData* countBuffer = [theCountBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glInterleavedArrays(GL_T2F_V3F, 0, NULL);
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
}

- (void)renderWireframePolygonsWithIndexBuffers:(NSMutableDictionary *)theIndexBuffers countBuffers:(NSMutableDictionary *)theCountBuffers {
    NSEnumerator* textureNameEn = [theIndexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        IntData* indexBuffer = [theIndexBuffers objectForKey:textureName];
        IntData* countBuffer = [theCountBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glVertexPointer(3, GL_FLOAT, 20, (const GLvoid *)8); // cast to pointer type to avoid compiler warning
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
}

- (void)renderFacesInMode:(ERenderMode)mode {
    
    if (!buffersValid) {
        NSEnumerator* renderEntityEn = [renderEntities objectEnumerator];
        RenderEntity* renderEntity;
        while ((renderEntity = [renderEntityEn nextObject])) {
            NSEnumerator* renderBrushEn = [[renderEntity renderBrushes] objectEnumerator];
            RenderBrush* renderBrush;
            while ((renderBrush = [renderBrushEn nextObject])) {
                Brush* brush = [renderBrush brush];
                NSArray* faces = [brush faces];
                
                NSEnumerator* faceEn = [faces objectEnumerator];
                Face* face;
                while ((face = [faceEn nextObject])) {
                    NSString* textureName = [face texture];
                    
                    NSMutableDictionary* currentIndexBuffers;
                    NSMutableDictionary* currentCountBuffers;
                    if ([selectionManager isBrushSelected:brush] || [selectionManager isFaceSelected:face]) {
                        currentIndexBuffers = selIndexBuffers;
                        currentCountBuffers = selCountBuffers;
                    } else {
                        currentIndexBuffers = indexBuffers;
                        currentCountBuffers = countBuffers;
                    }
                    
                    IntData* indexBuffer = [currentIndexBuffers objectForKey:textureName];
                    if (indexBuffer == nil) {
                        indexBuffer = [[IntData alloc] init];
                        [currentIndexBuffers setObject:indexBuffer forKey:textureName];
                        [indexBuffer release];
                    }
                    
                    IntData* countBuffer = [currentCountBuffers objectForKey:textureName];
                    if (countBuffer == nil) {
                        countBuffer = [[IntData alloc] init];
                        [currentCountBuffers setObject:countBuffer forKey:textureName];
                        [countBuffer release];
                    }
                    
                    [renderBrush indexForFace:face indexBuffer:indexBuffer countBuffer:countBuffer];
                }
            }
        }
        
        buffersValid = YES;
    }
    
    switch (mode) {
        case RM_TEXTURED:
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            
            glColor4f(0.7, 0.7, 0.7, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            [self renderTexturedPolygonsWithIndexBuffers:indexBuffers countBuffers:countBuffers];
            
            glColor4f(1, 0, 0, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            [self renderTexturedPolygonsWithIndexBuffers:selIndexBuffers countBuffers:selCountBuffers];
            break;
        case RM_FLAT:
            break;
        case RM_WIREFRAME:
            glDisable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_LINE);
            
            glColor4f(1, 1, 1, 0.5);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            [self renderWireframePolygonsWithIndexBuffers:indexBuffers countBuffers:countBuffers];
            
            glDisable(GL_DEPTH_TEST);
            glColor4f(1, 0, 0, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            [self renderWireframePolygonsWithIndexBuffers:selIndexBuffers countBuffers:selCountBuffers];
            glEnable(GL_DEPTH_TEST);
            break;
    }
}

- (void)postRender {
    [faceVBO deactivate];
}

- (void)render {
    [self preRender];
    if (!buffersValid)
        [self prepareFaces];
    
    [self renderFacesInMode:RM_TEXTURED];
    [self renderFacesInMode:RM_WIREFRAME];
    
    [self postRender];
}

- (void)updateView:(NSRect)bounds {
    float fov = [camera fieldOfVision];
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
    [indexBuffers release];
    [countBuffers release];
    [selIndexBuffers release];
    [selCountBuffers release];
    [camera release];
    [textureManager release];
    [selectionManager release];
    [faceVBO release];
    [renderEntities release];
    [map release];
    [super dealloc];
}

@end
