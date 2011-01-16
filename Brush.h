//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;
@class Face;

extern NSString* const BrushFaceAdded;
extern NSString* const BrushFaceRemoved;
extern NSString* const BrushFaceChanged;

@interface Brush : NSObject {
    @private
	NSMutableSet* faces;
}

- (id)initCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions;
- (void)faceChanged:(NSNotification *)notification;
- (void)registerAsObserverOf:(Face *)face;
- (void)deregisterAsObserverOf:(Face *)face;

- (NSSet *)faces;
- (NSSet *)polygons;
@end
