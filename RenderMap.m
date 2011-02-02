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
#import "VBOBuffer.h"

@implementation RenderMap

- (id)init {
    if (self = [super init]) {
        map = nil;
        renderEntities = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addEntity:(Entity *)theEntity {
    RenderEntity* renderEntity = [[RenderEntity alloc] initWithEntity:theEntity faceVBO:faceVBO];
    [renderEntities setObject:renderEntity forKey:[theEntity entityId]];
    [renderEntity release];
}

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];
    [self addEntity:entity];
}

- (void)entityRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];
    [renderEntities removeObjectForKey:[entity entityId]];
}

- (id)initWithMap:(Map *)theMap faceVBO:(VBOBuffer *)theFaceVBO {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];

    if (self = [self init]) {
        map = [theMap retain];
        faceVBO = [theFaceVBO retain];
        
        NSArray* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        
        while ((entity = [entityEn nextObject]))
            [self addEntity:entity];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded object:map];
        [center addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved object:map];
    }
    
    return self;
}

- (NSArray *)renderEntities {
    return [renderEntities allValues];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceVBO release];
    [renderEntities release];
    [map release];
    [super dealloc];
}

@end
