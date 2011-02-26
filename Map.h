//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@protocol Map <NSObject>

- (Entity *)worldspawn;

- (Entity *)createEntity;
- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value;
- (void)removeEntity:(Entity *)entity;

- (NSArray* )entities;

- (BOOL)postNotifications;

- (void)faceFlagsChanged:(Face *)face;
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture;
- (void)faceGeometryChanged:(Face *)face;
- (void)faceAdded:(Face *)face;
- (void)faceRemoved:(Face *)face;

- (void)brushAdded:(Brush *)brush;
- (void)brushRemoved:(Brush *)brush;

- (void)propertyAdded:(Entity *)entity key:(NSString *)key value:(NSString *)value;
- (void)propertyRemoved:(Entity *)entity key:(NSString *)key value:(NSString *)value;
- (void)propertyChanged:(Entity *)entity key:(NSString *)key oldValue:(NSString *)oldValue newValue:(NSString *)newValue;

@end
