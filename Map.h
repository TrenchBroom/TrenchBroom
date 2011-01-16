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

extern NSString* const MapEntity;

@class Entity;

@interface Map : NSObject {
    @private
    Entity* worldspawn;
    NSMutableSet* entities;
}

- (Entity *)worldspawn;

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value;
- (void)removeEntity:(Entity *)entity;

- (NSSet* )entities;

@end
