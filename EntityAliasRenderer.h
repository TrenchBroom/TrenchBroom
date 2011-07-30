//
//  AliasRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class EntityRendererManager;
@protocol Entity;
@protocol Filter;

@interface EntityAliasRenderer : NSObject {
    EntityRendererManager* entityRendererManager;
    NSMutableSet* entities;
    NSMutableSet* invalidEntities;
    NSMutableDictionary* entityRenderers;
    id <Filter> filter;
    NSArray* mods;
    BOOL cacheValid;
}

- (id)initWithEntityRendererManager:(EntityRendererManager *)theEntityRendererManager;

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)render;

- (void)setFilter:(id <Filter>)theFilter;
- (void)setMods:(NSArray *)theMods;
- (void)refreshRendererCache;
@end
