//
//  RenderMap.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderMap.h"
#import "Map.h"
#import "Entity.h"
#import "RenderEntity.h"

@implementation RenderMap

- (id)init {
    if (self = [super init]) {
        map = nil;
        renderEntities = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithMap:(Map *)aMap {
    if (aMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];

    if (self = [self init]) {
        map = [aMap retain];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded object:map];
        [center addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved object:map];
        
        NSSet* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        
        while ((entity = [entityEn nextObject])) {
            RenderEntity* renderEntity = [[RenderEntity alloc] initWithEntity:entity];
            [renderEntities setObject:renderEntity forKey:[entity getId]];
            [renderEntity release];
        }
    }
    
    return self;
}

- (void)entityAdded:(NSNotification *)notification {
    Entity* entity = [notification object];
    RenderEntity* renderEntity = [[RenderEntity alloc] initWithEntity:entity];
    [renderEntities setObject:renderEntity forKey:[entity getId]];
    [renderEntity release];
}

- (void)entityRemoved:(NSNotification *)notification {
    Entity* entity = [notification object];
    [renderEntities removeObjectForKey:[entity getId]];
}

- (void)renderWithContext:(id <RenderContext>)renderContext {
    if (renderContext == nil)
        [NSException raise:NSInvalidArgumentException format:@"render context must not be nil"];
    
    NSEnumerator* en = [renderEntities objectEnumerator];
    RenderEntity* entity;
    
    while ((entity = [en nextObject]))
        [entity renderWithContext:renderContext];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [renderEntities release];
    [map release];
    [super dealloc];
}

@end
