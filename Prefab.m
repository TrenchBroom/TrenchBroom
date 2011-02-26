//
//  Prefab.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Prefab.h"
#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@implementation Prefab

- (id)init {
    if (self = [super init]) {
        entity = [[Entity alloc] initInMap:self];
    }
    
    return self;
}

- (Entity *)worldspawn {
    return entity;
}

- (Entity *)createEntity {
    return entity;
}

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value {
    return entity;
}

- (void)removeEntity:(Entity *)entity {
}

- (NSArray* )entities {
    return [NSArray arrayWithObject:entity];
}

- (BOOL)postNotifications {
    return NO;
}

- (void)faceFlagsChanged:(Face *)face {}
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture {}
- (void)faceGeometryChanged:(Face *)face {}
- (void)faceAdded:(Face *)face {}
- (void)faceRemoved:(Face *)face {}

- (void)brushAdded:(Brush *)brush {}
- (void)brushRemoved:(Brush *)brush {}

- (void)propertyAdded:(Entity *)entity key:(NSString *)key value:(NSString *)value {}
- (void)propertyRemoved:(Entity *)entity key:(NSString *)key value:(NSString *)value {}
- (void)propertyChanged:(Entity *)entity key:(NSString *)key oldValue:(NSString *)oldValue newValue:(NSString *)newValue {}

@end
