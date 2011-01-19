//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;
@class Brush;

@interface Entity : NSObject {
    @private
    NSNumber* entityId;
	NSMutableArray* brushes;
	NSMutableDictionary* properties;
}

- (id)initWithProperty:(NSString *)key value:(NSString *)value;

- (Brush *)createCuboidAt:(Vector3i *)position dimensions:(Vector3i *)dimensions texture:(NSString *)texture;
- (Brush *)createBrush;

- (NSNumber *)getId;

- (NSArray *)brushes;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;
- (NSString *)propertyForKey:(NSString *)key;
- (NSString *)classname;

- (NSDictionary *)properties;

- (BOOL)isWorldspawn;

@end
