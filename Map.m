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
#import "Face.h"

NSString* const MapEntityAdded      = @"MapEntityAdded";
NSString* const MapEntityRemoved    = @"MapEntityRemoved";
NSString* const EntityKey           = @"Entity";

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
    [[undoManager prepareWithInvocationTarget:self] removeEntity:theEntity];
    
    [entities addObject:theEntity];
    [self addForward:FaceGeometryChanged from:theEntity];
    [self addForward:FaceFlagsChanged from:theEntity];
    [self addForward:BrushFaceAdded from:theEntity];
    [self addForward:BrushFaceRemoved from:theEntity];
    [self addForward:EntityBrushAdded from:theEntity];
    [self addForward:EntityBrushRemoved from:theEntity];
    [self addForward:EntityPropertyAdded from:theEntity];
    [self addForward:EntityPropertyChanged from:theEntity];
    [self addForward:EntityPropertyRemoved from:theEntity];
    
    [self notifyObservers:MapEntityAdded infoObject:theEntity infoKey:EntityKey];
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
    [[undoManager prepareWithInvocationTarget:self] addEntity:entity];
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:entity forKey:EntityKey];
    [entity removeObserver:self];
    
    [entities removeObject:entity];
    [self notifyObservers:MapEntityAdded userInfo:userInfo];
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

- (NSUndoManager *)undoManager {
    return undoManager;
}

- (void)setUndoManager:(NSUndoManager *)theUndoManager {
    [undoManager release];
    undoManager = [theUndoManager retain];
}

- (void)dealloc {
    NSEnumerator* entityEn = [entities objectEnumerator];
    Entity* entity;
    while ((entity = [entityEn nextObject]))
        [entity removeObserver:self];
    
    [undoManager release];
    [entities release];
    [super dealloc];
}

@end
