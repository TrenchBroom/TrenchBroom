//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Entity;

@interface Map : NSObject {
    @private
    NSMutableArray* entities;
    Entity* worldspawn;
    int worldSize;
}

- (Entity *)worldspawn;

- (Entity *)createEntity;
- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value;
- (void)removeEntity:(Entity *)entity;

- (NSArray* )entities;
- (int)worldSize;
@end
