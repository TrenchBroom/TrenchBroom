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

@interface Entity : NSObject {
	NSMutableSet* brushes;
	NSMutableDictionary* properties;
}

- (id)initWithKey:(NSString *)key value:(NSString *)value;

- (Brush *)createCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions;

- (void)addBrush:(Brush *)brush;
- (void)removeBrush:(Brush *)brush;


@end
