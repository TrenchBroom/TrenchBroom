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

@implementation Map

- (id)init {
    if (self = [super init]) {
        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
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

- (Entity *)createEntity {
    Entity* entity = [[Entity alloc] init];
    [entities addObject:entity];
    return [entity autorelease];
}

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value {
    Entity* entity = [[Entity alloc] initWithProperty:key value:value];
    [entities addObject:entity];
    return entity;
}

- (void)removeEntity:(Entity *)entity {
    [entities removeObject:entity];
}

- (NSArray *)entities {
    return entities;
}

- (void)dealloc {
    [entities release];
    [super dealloc];
}

@end
