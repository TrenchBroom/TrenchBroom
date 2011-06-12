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
#import "Entity.h"
#import "Options.h"

@implementation DefaultEntityLayer

- (id)initWithOptions:(Options *)theOptions {
    if ((self = [self init])) {
        options = [theOptions retain];
        boundsRenderer = [[EntityBoundsRenderer alloc] init];
        aliasRenderer = [[EntityAliasRenderer alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [boundsRenderer release];
    [aliasRenderer release];
    [options release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    [boundsRenderer addEntity:entity];
    [aliasRenderer addEntity:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [boundsRenderer removeEntity:entity];
    [aliasRenderer removeEntity:entity];
}

- (void)updateEntity:(id <Entity>)entity {
    [self removeEntity:entity];
    [self addEntity:entity];
}

- (void)render {
    if ([options renderEntities]) {
        if ([options isolationMode] == IM_NONE) {
            [boundsRenderer renderWithColor:YES];
            [aliasRenderer render];
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

@end
