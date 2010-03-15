//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3i.h"

@interface Face : NSObject {
	Vector3i* point1;
	Vector3i* point2;
	Vector3i* point3;
	
	NSString* texture;
	int xOffset;
	int yOffset;
	float rotation;
	float xScale;
	float yScale;
}

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
- (void)setYSCale:(float)factor;
@end
