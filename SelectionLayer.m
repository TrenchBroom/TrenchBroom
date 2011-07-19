//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionLayer.h"
#import "Face.h"
#import "Entity.h"
#import "BoundsRenderer.h"
#import "EntityBoundsRenderer.h"
#import "EntityAliasRenderer.h"
#import "TextRenderer.h"
#import "EntityClassnameAnchor.h"
#import "Options.h"
#import "GLUtils.h"
#import "SelectionManager.h"
#import "GLResources.h"

@interface SelectionLayer (private)

- (void)validateEntities;

@end

@implementation SelectionLayer (private)

- (void)validateEntities {
    if ([removedEntities count] > 0) {
        NSEnumerator* entityEn = [removedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [entityBoundsRenderer removeEntity:entity];
            [entityAliasRenderer removeEntity:entity];
            [entityClassnameRenderer removeStringForKey:[entity entityId]];
        }
        
        [removedEntities removeAllObjects];
    }

    if ([addedEntities count] > 0) {
        NSEnumerator* entityEn = [addedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [entityBoundsRenderer addEntity:entity];
            [entityAliasRenderer addEntity:entity];
        }
        
        [fontManager activate];
        entityEn = [addedEntities objectEnumerator];
        while ((entity = [entityEn nextObject])) {
            NSString* classname = [entity classname];
            EntityClassnameAnchor* anchor = [[EntityClassnameAnchor alloc] initWithEntity:entity];
            [entityClassnameRenderer addString:classname forKey:[entity entityId] withFont:[NSFont systemFontOfSize:9] withAnchor:anchor];
            [anchor release];
        }
        [fontManager deactivate];
        
        [addedEntities removeAllObjects];
    }
}


@end

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo glResources:(GLResources *)theGLResources selectionManager:(SelectionManager *)theSelectionManager options:(Options *)theOptions camera:(Camera *)theCamera {
    NSAssert(theVbo != nil, @"VBO must not be nil");
    NSAssert(theGLResources != nil, @"GL resources must not be nil");
    NSAssert(theSelectionManager != nil, @"selection manager must not be nil");
    NSAssert(theOptions != nil, @"options must not be nil");
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    if ((self = [super initWithVbo:theVbo textureManager:[theGLResources textureManager] options:theOptions])) {
        selectionManager = [theSelectionManager retain];
        camera = [theCamera retain];
        fontManager = [[theGLResources fontManager] retain];
        
        brushBoundsRenderer = [[BoundsRenderer alloc] initWithCamera:camera fontManager:fontManager];
        entityBoundsRenderer = [[EntityBoundsRenderer alloc] init];
        entityAliasRenderer = [[EntityAliasRenderer alloc] initWithEntityRendererManager:[theGLResources entityRendererManager]];
        entityClassnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:camera];
        
        addedEntities = [[NSMutableSet alloc] init];
        removedEntities = [[NSMutableSet alloc] init];
    }
    
    return self;
}


- (void)dealloc {
    [brushBoundsRenderer release];
    [entityBoundsRenderer release];
    [entityAliasRenderer release];
    [entityClassnameRenderer release];
    [addedEntities release];
    [removedEntities release];
    [fontManager release];
    [camera release];
    [selectionManager release];
    [super dealloc];
}

- (void)preRenderEdges {
    if (edgePass == 1) {
        glSetEdgeOffset(0.5);
        glColor4f(1, 0, 0, 0.2f);
        glDisable(GL_DEPTH_TEST);
    } else if (edgePass == 2) {
        glSetEdgeOffset(1);
        glColor4f(1, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glResetEdgeOffset();
}

- (void)setTextureMode {
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

- (void)setFaceColorMode {
//    glColor4f(1, 0, 0, 1);
}

- (BOOL)doRenderFaces {
    return YES;
}

- (BOOL)doRenderEdges {
    return YES;
}

- (void)addEntity:(id <Entity>)entity {
    [addedEntities addObject:entity];
    
    if ([selectionManager mode] == SM_BRUSHES_ENTITIES) {
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        [brushBoundsRenderer setBounds:&bounds];
    }
}

- (void)removeEntity:(id <Entity>)entity {
    if ([addedEntities containsObject:entity])
        [addedEntities removeObject:entity];
    else
        [removedEntities addObject:entity];

    if ([selectionManager mode] == SM_BRUSHES_ENTITIES || [selectionManager mode] == SM_BRUSHES) {
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        [brushBoundsRenderer setBounds:&bounds];
    }
}

- (void)updateEntity:(id <Entity>)entity {
    [self removeEntity:entity];
    [self addEntity:entity];
}

- (void)addBrush:(id<Brush>)theBrush {
    [super addBrush:theBrush];
    
    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    [brushBoundsRenderer setBounds:&bounds];
}

- (void)removeBrush:(id<Brush>)theBrush {
    [super removeBrush:theBrush];

    if ([selectionManager mode] == SM_BRUSHES_ENTITIES || [selectionManager mode] == SM_BRUSHES) {
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        [brushBoundsRenderer setBounds:&bounds];
    }
}

- (void)addFace:(id<Face>)theFace {
    [super addFace:theFace];

    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    [brushBoundsRenderer setBounds:&bounds];
}

- (void)removeFace:(id<Face>)theFace {
    [super removeFace:theFace];
    
    if ([selectionManager mode] == SM_FACES) {
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        [brushBoundsRenderer setBounds:&bounds];
    }
}

- (void)render {
    edgePass = 1;
    [super render];
    
    [sharedVbo activate];
    edgePass = 2;
    [super renderEdges];
    [sharedVbo deactivate];
    
    glDisable(GL_DEPTH_TEST);
    float col[] = {1, 0, 0, 0.5f};
    [brushBoundsRenderer renderColor:col];
    glEnable(GL_DEPTH_TEST);
    col[3] = 1;
    [brushBoundsRenderer renderColor:col];

    if ([options renderEntities]) {
        [self validateEntities];
        
//        glColor4f(1, 0, 0, 1);
//        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        [entityAliasRenderer render];

        [fontManager activate];

        glDisable(GL_DEPTH_TEST);
        col[3] = 0.5f;
        [entityClassnameRenderer renderColor:col];

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        col[3] = 1;
        [entityClassnameRenderer renderColor:col];
        glDepthFunc(GL_LESS);

        [fontManager deactivate];

        glSetEdgeOffset(0.5);
        glColor4f(1, 0, 0, 0.2f);
        glDisable(GL_DEPTH_TEST);
        [entityBoundsRenderer renderWithColor:NO];

        glSetEdgeOffset(1);
        glColor4f(1, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        [entityBoundsRenderer renderWithColor:NO];

        glDepthFunc(GL_LESS);
        glResetEdgeOffset();
    }
}

- (void)setFilter:(id <Filter>)theFilter {
    [super setFilter:theFilter];
    [entityBoundsRenderer setFilter:theFilter];
}

@end
