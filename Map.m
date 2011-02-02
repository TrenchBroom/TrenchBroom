//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Map.h"
#import "Entity.h"
#import "Brush.h"

NSString* const MapEntityAdded      = @"MapEntityAdded";
NSString* const MapEntityRemoved    = @"MapEntityRemoved";
NSString* const MapEntityKey        = @"MapEntity";

@implementation Map

- (id)init {
    if (self = [super init]) {
        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
        worldSize = 8192;
        postNotifications = YES;
    }
    
    return self;
}

- (Entity *)worldspawn {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        NSEnumerator* en = [entities objectEnumerator];
        while ((worldspawn = [en nextObject]))
            if ([worldspawn isWorldspawn])
                break;
    }
    
    return worldspawn;
}

- (void)addEntity:(Entity *)theEntity {
    [entities addObject:theEntity];
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:MapEntityAdded object:self userInfo:[NSDictionary dictionaryWithObject:theEntity forKey:MapEntityKey]];
    }
}

- (Entity *)createEntity {
    Entity* entity = [[Entity alloc] initInMap:self];
    [self addEntity:entity];
    return [entity autorelease];
}

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value {
    Entity* entity = [[Entity alloc] initInMap:self property:key value:value];
    [self addEntity:entity];
    return entity;
}

- (void)removeEntity:(Entity *)entity {
    [entities removeObject:entity];
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:MapEntityRemoved object:self userInfo:[NSDictionary dictionaryWithObject:entity forKey:MapEntityKey]];
    }
}

- (NSArray *)entities {
    return entities;
}

- (int)worldSize {
    return worldSize;
}

- (BOOL)postNotifications {
    return postNotifications;
}

- (void)setPostNotifications:(BOOL)value {
    postNotifications = value;
}

- (void)dealloc {
    [entities release];
    [super dealloc];
}

@end
