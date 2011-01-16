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

NSString* const MapEntityAdded = @"EntityAdded";
NSString* const MapEntityRemoved = @"EntityRemoved";

NSString* const MapEntity = @"Entity";

@implementation Map

- (id)init {
    if (self = [super init]) {
        worldspawn = [[Entity alloc] initWithProperty:@"classname" value:@"worldspawn"];
        entities = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (Entity *)worldspawn {
    return worldspawn;
}

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value {
    Entity* entity = [[Entity alloc] initWithProperty:key value:value];
    [entities addObject:entity];
    [entity release];

    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:1];
    [info setObject:entity forKey:MapEntity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:MapEntityAdded object:self userInfo:info];
    return entity;
}

- (void)removeEntity:(Entity *)entity {
    [entities removeObject:entity];

    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:1];
    [info setObject:entity forKey:MapEntity];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:MapEntityRemoved object:self userInfo:info];
}

- (NSSet *)entities {
    return entities;
}

- (void)dealloc {
    [entities release];
    [worldspawn release];
    
    [super dealloc];
}

@end
