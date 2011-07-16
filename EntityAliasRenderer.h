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
    NSMutableDictionary* entityRenderers;
    id <Filter> filter;
}

- (id)initWithEntityRendererManager:(EntityRendererManager *)theEntityRendererManager;

- (void)addEntity:(id <Entity>)entity;
- (void)removeEntity:(id <Entity>)entity;

- (void)render;

- (void)setFilter:(id <Filter>)theFilter;
@end
