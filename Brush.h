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
    NSNumber* brushId;
	NSMutableSet* faces;
}

- (id)initCuboidAt:(Vector3i *)position dimensions:(Vector3i *)dimensions texture:(NSString *)texture;
- (void)faceChanged:(NSNotification *)notification;
- (void)registerAsObserverOf:(Face *)face;
- (void)deregisterAsObserverOf:(Face *)face;

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture;

- (NSNumber* )getId;
- (NSSet *)faces;
- (NSSet *)polygons;
@end
