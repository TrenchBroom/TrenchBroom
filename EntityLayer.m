//
//  EntityLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityLayer.h"
#import "EntityBoundsRenderer.h"
#import "Entity.h"

@implementation EntityLayer

- (id)initWithEntityDefinitionManager:(EntityDefinitionManager *)theDefinitionManager {
    NSAssert(theDefinitionManager != nil, @"entity definition manager must not be nil");
    
    if (self = [self init]) {
        definitionManager = [theDefinitionManager retain];
        boundsRenderer = [[EntityBoundsRenderer alloc] initWithEntityDefinitionManager:definitionManager];
    }
    
    return self;
}

- (void)addEntity:(id <Entity>)entity {
    [boundsRenderer addEntity:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [boundsRenderer removeEntity:entity];
}

- (void)updateEntity:(id <Entity>)entity {
    [boundsRenderer removeEntity:entity];
    [boundsRenderer addEntity:entity];
}

- (void)render:(RenderContext *)renderContext {
    [boundsRenderer render];
}

- (void)dealloc {
    [boundsRenderer release];
    [definitionManager release];
    [super dealloc];
}

@end
