//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    XY, XZ, YZ
} EPlaneType;

@class Vector2f;
@class Vector3f;
@class Vector3i;
@class HalfSpace3D;

@interface Face : NSObject {
    @private
    NSNumber* faceId;
	Vector3i* point1;
	Vector3i* point2;
	Vector3i* point3;
	
	NSMutableString* texture;
	int xOffset;
	int yOffset;
	float rotation;
	float xScale;
	float yScale;
    
    Vector3f* norm;
    Vector3f* texAxisX;
    Vector3f* texAxisY;
}

- (id)initOnPlane:(EPlaneType)plane at:(Vector3i *)position thirdAxisPositive:(BOOL)thirdAxisPositive texture:(NSString *)texture;
- (id)initWithPoint1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture;

- (NSNumber *)getId;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;

- (NSString *)texture;
- (int)xOffset;
- (int)yOffset;
- (float)rotation;
- (float)xScale;
- (float)yScale;

- (Vector3f *)norm;

- (void)setPoint1:(Vector3i *)point;
- (void)setPoint2:(Vector3i *)point;
- (void)setPoint3:(Vector3i *)point;

- (void)setTexture:(NSString *)name;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex;

- (HalfSpace3D *)halfSpace;
@end
