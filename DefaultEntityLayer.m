//
//  EntityLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultEntityLayer.h"
#import "EntityBoundsRenderer.h"
#import "EntityAliasRenderer.h"
#import "TextRenderer.h"
#import "EntityClassnameAnchor.h"
#import "Entity.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "GLResources.h"
#import "GLFontManager.h"
#import "Camera.h"
#import "Options.h"

@interface DefaultEntityLayer (private)

- (void)validate;

@end

@implementation DefaultEntityLayer (private)

- (void)validate {
    if ([removedEntities count] > 0) {
        NSEnumerator* entityEn = [removedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [boundsRenderer removeEntity:entity];
            [aliasRenderer removeEntity:entity];
            [classnameRenderer removeStringForKey:[entity entityId]];
        }
        
        [removedEntities removeAllObjects];
    }

    if ([addedEntities count] > 0) {
        NSEnumerator* entityEn = [addedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [boundsRenderer addEntity:entity];
            [aliasRenderer addEntity:entity];
        }
        
        [fontManager activate];
        entityEn = [addedEntities objectEnumerator];
        while ((entity = [entityEn nextObject])) {
            NSString* classname = [entity classname];
            EntityClassnameAnchor* anchor = [[EntityClassnameAnchor alloc] initWithEntity:entity];
            [classnameRenderer addString:classname forKey:[entity entityId] withFont:[NSFont systemFontOfSize:9] withAnchor:anchor];
            [anchor release];
        }
        [fontManager deactivate];
        
        [addedEntities removeAllObjects];
    }
}

@end

@implementation DefaultEntityLayer

- (id)initWithGLResources:(GLResources *)theGLResources camera:(Camera *)theCamera options:(Options *)theOptions {
    NSAssert(theGLResources != nil, @"GL resources must not be nil");
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theOptions != nil, @"options must not be nil");
    
    if ((self = [self init])) {
        addedEntities = [[NSMutableSet alloc] init];
        removedEntities = [[NSMutableSet alloc] init];
        
        options = [theOptions retain];
        fontManager = [[theGLResources fontManager] retain];
        boundsRenderer = [[EntityBoundsRenderer alloc] init];
        aliasRenderer = [[EntityAliasRenderer alloc] initWithEntityRendererManager:[theGLResources entityRendererManager]];
        classnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:theCamera];
    }
    
    return self;
}

- (void)dealloc {
    [addedEntities release];
    [removedEntities release];
    [options release];
    [boundsRenderer release];
    [aliasRenderer release];
    [classnameRenderer release];
    [fontManager release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    [addedEntities addObject:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    if ([addedEntities containsObject:entity])
        [addedEntities removeObject:entity];
    else
        [removedEntities addObject:entity];
}

- (void)updateEntity:(id <Entity>)entity {
    [self removeEntity:entity];
    [self addEntity:entity];
}

- (void)render {
    if ([options renderEntities]) {
        [self validate];
        
        if ([options isolationMode] == IM_NONE) {
            [boundsRenderer renderWithColor:YES];
            [aliasRenderer render];

            if ([options renderEntityClassnames]) {
                [fontManager activate];
                float col[] = {1, 1, 1, 1};
                [classnameRenderer renderColor:col];
                [fontManager deactivate];
            }
        } else if ([options isolationMode] == IM_WIREFRAME) {
            glColor4f(1, 1, 1, 0.6f);
            [boundsRenderer renderWithColor:NO];
        }
    }
}

- (void)setFilter:(id <Filter>)theFilter {
    [boundsRenderer setFilter:theFilter];
    [aliasRenderer setFilter:theFilter];
}

- (void)setMods:(NSArray *)theMods {
    [aliasRenderer setMods:theMods];
}

- (void)refreshRendererCache {
    [aliasRenderer refreshRendererCache];
}


@end
