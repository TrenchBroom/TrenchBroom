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

- (id)init {
    if (self = [super init]) {
        boundsRenderer = [[EntityBoundsRenderer alloc] init];
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
    [super dealloc];
}

@end
