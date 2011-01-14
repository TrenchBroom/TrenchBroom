//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;

extern NSString* const BrushFaceAdded;
extern NSString* const BrushFaceRemoved;

@interface Brush : NSObject {
    @private
	NSMutableSet* faces;
}

- (id)initCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions;

- (NSSet *)faces;
- (NSSet *)polygons;
@end
