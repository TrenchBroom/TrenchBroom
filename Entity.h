//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3i.h"
#import "Brush.h"

extern NSString* const EntityBrushAdded;
extern NSString* const EntityBrushRemoved;

extern NSString* const EntityBrush;

extern NSString* const EntityPropertyAdded;
extern NSString* const EntityPropertyRemoved;
extern NSString* const EntityPropertyChanged;

extern NSString* const EntityPropertyKey;
extern NSString* const EntityPropertyNewValue;
extern NSString* const EntityPropertyOldValue;

@interface Entity : NSObject {
	NSMutableSet* brushes;
	NSMutableDictionary* properties;
}

- (id)initWithProperty:(NSString *)key value:(NSString *)value;

- (Brush *)createCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions;

- (void)addBrush:(Brush *)brush;
- (void)removeBrush:(Brush *)brush;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;
- (NSString *)propertyForKey:(NSString *)key;

@end
