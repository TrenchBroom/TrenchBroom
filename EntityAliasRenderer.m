//
//  AliasRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityAliasRenderer.h"
#import "Entity.h"
#import "EntityRendererManager.h"
#import "EntityRenderer.h"
#import "BspRenderer.h"
#import "AliasRenderer.h"
#import "Filter.h"

@implementation EntityAliasRenderer

- (id)initWithEntityRendererManager:(EntityRendererManager *)theEntityRendererManager {
    NSAssert(theEntityRendererManager != nil, @"entity renderer manager must not be nil");
    
    if ((self = [super init])) {
        entityRendererManager = [theEntityRendererManager retain];
        entities = [[NSMutableSet alloc] init];
        invalidEntities = [[NSMutableSet alloc] init];
        entityRenderers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [invalidEntities release];
    [entityRendererManager release];
    [entities release];
    [entityRenderers release];
    [filter release];
    [mods release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must no be nil");
    [invalidEntities addObject:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [entityRenderers removeObjectForKey:[entity entityId]];
    [entities removeObject:entity];
    [invalidEntities removeObject:entity];
}

- (void)render {
    if ([invalidEntities count] > 0) {
        NSEnumerator* entityEn = [invalidEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
            if (renderer != nil) {
                [entities addObject:entity];
                [entityRenderers setObject:renderer forKey:[entity entityId]];
            }
        }
        
        [invalidEntities removeAllObjects];
    }
    
    [entityRendererManager activate];
    
    glMatrixMode(GL_MODELVIEW);
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if (filter == nil || [filter isEntityRenderable:entity]) {
            id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:[entity entityId]];

            glPushMatrix();
            [entityRenderer renderWithEntity:entity];
            glPopMatrix();
        }
    }
    
    glDisable(GL_TEXTURE_2D);
    [entityRendererManager deactivate];
}

- (void)setFilter:(id <Filter>)theFilter {
    [filter release];
    filter = [theFilter retain];
}

- (void)setMods:(NSArray *)theMods {
    NSAssert(theMods != nil, @"mod list must not be nil");
    
    if ([theMods isEqualToArray:mods])
        return;
    
    [mods release];
    mods = [theMods retain];
    [self refreshRendererCache];
}

- (void)refreshRendererCache {
    [entityRenderers removeAllObjects];
    [invalidEntities setSet:entities];
}

@end
