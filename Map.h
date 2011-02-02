//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const MapEntityAdded;
extern NSString* const MapEntityRemoved;
extern NSString* const MapEntityKey;

@class Entity;

@interface Map : NSObject {
    @private
    NSMutableArray* entities;
    Entity* worldspawn;
    int worldSize;
    BOOL postNotifications;
}

- (Entity *)worldspawn;

- (Entity *)createEntity;
- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value;
- (void)removeEntity:(Entity *)entity;

- (NSArray* )entities;
- (int)worldSize;

- (BOOL)postNotifications;
- (void)setPostNotifications:(BOOL)value;
@end
