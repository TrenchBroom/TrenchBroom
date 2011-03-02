//
//  MathCache.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class Vector3i;
@class Quaternion;
@class Line3D;
@class Plane3D;

@interface MathCache : NSObject {
    @private
    NSMutableArray* vector3fCache;
    NSMutableArray* vector3iCache;
    NSMutableArray* quaternionCache;
    NSMutableArray* line3DCache;
    NSMutableArray* plane3DCache;
}

+ (MathCache *)sharedCache;

- (Vector3f *)vector3f;
- (void)returnVector3f:(Vector3f *)vector;

- (Vector3i *)vector3i;
- (void)returnVector3i:(Vector3i *)vector;

- (Quaternion *)quaternion;
- (void)returnQuaternion:(Quaternion *)quaternion;

- (Line3D *)line3D;
- (void)returnLine3D:(Line3D *)line;

- (Plane3D *)plane3D;
- (void)returnPlane3D:(Plane3D *)plane;

@end
