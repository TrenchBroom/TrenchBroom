//
//  EntityLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultEntityLayer.h"
#import "EntityBoundsRenderer.h"
#import "Entity.h"
#import "Options.h"

@implementation DefaultEntityLayer

- (id)initWithOptions:(Options *)theOptions {
    if (self = [self init]) {
        options = [theOptions retain];
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

- (void)render {
    if ([options renderEntities])
        [boundsRenderer renderWithColor:YES];
}

- (void)setFilter:(id <Filter>)theFilter {
    [boundsRenderer setFilter:theFilter];
}

- (void)dealloc {
    [boundsRenderer release];
    [options release];
    [super dealloc];
}

@end
