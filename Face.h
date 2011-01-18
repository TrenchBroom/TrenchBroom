//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const FacePoint1Changed;
extern NSString* const FacePoint2Changed;
extern NSString* const FacePoint3Changed;
extern NSString* const FaceTextureChanged;
extern NSString* const FaceXOffsetChanged;
extern NSString* const FaceYOffsetChanged;
extern NSString* const FaceRotationChanged;
extern NSString* const FaceXScaleChanged;
extern NSString* const FaceYScaleChanged;

extern NSString* const FacePoint1Old;
extern NSString* const FacePoint1New;
extern NSString* const FacePoint2Old;
extern NSString* const FacePoint2New;
extern NSString* const FacePoint3Old;
extern NSString* const FacePoint3New;
extern NSString* const FaceTextureOld;
extern NSString* const FaceTextureNew;
extern NSString* const FaceXOffsetOld;
extern NSString* const FaceXOffsetNew;
extern NSString* const FaceYOffsetOld;
extern NSString* const FaceYOffsetNew;
extern NSString* const FaceRotationOld;
extern NSString* const FaceRotationNew;
extern NSString* const FaceXScaleOld;
extern NSString* const FaceXScaleNew;
extern NSString* const FaceYScaleOld;
extern NSString* const FaceYScaleNew;

typedef enum {
    XY, XZ, YZ
} EPlaneType;

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

- (void)setPoint1:(Vector3i *)point;
- (void)setPoint2:(Vector3i *)point;
- (void)setPoint3:(Vector3i *)point;

- (void)setTexture:(NSString *)name;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;

- (HalfSpace3D *)halfSpace;
@end
