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
@class Polyhedron;

@interface Brush : NSObject {
    @private
    NSNumber* brushId;
	NSMutableArray* faces;
    NSMutableDictionary* vertices;
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture;

- (NSNumber* )getId;
- (NSArray *)faces;
- (NSArray *)verticesForFace:(Face *)face;
@end
