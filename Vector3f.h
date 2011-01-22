//
//  Vector3f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3i;

@interface Vector3f : NSObject {
    @private
	float x;
	float y;
	float z;
}

+ (Vector3f *)add:(Vector3f *)left addend:(Vector3f *)right;
+ (Vector3f *)sub:(Vector3f *)left subtrahend:(Vector3f *)right;
+ (Vector3f *)cross:(Vector3f *)left factor:(Vector3f *)right;
+ (Vector3f *)normalize:(Vector3f *)vector;

+ (Vector3f *)vector;
+ (Vector3f *)vectorWithFloatVector:(Vector3f *)vector;
+ (Vector3f *)vectorWithIntVector:(Vector3i *)vector;
+ (Vector3f *)vectorWithX:(float)xCoord y:(float)yCoord z:(float)zCoord;

+ (Vector3f *)xAxisPos;
+ (Vector3f *)xAxisNeg;
+ (Vector3f *)yAxisPos;
+ (Vector3f *)yAxisNeg;
+ (Vector3f *)zAxisPos;
+ (Vector3f *)zAxisNeg;

- (id)initWithFloatVector:(Vector3f *)vector;
- (id)initWithIntVector:(Vector3i *)vector;
- (id)initWithX:(float)xCoord y:(float)yCoord z:(float)zCoord;

- (float)x;
- (float)y;
- (float)z;

- (float)component:(int)index;
- (void)setComponent:(int)index value:(float)value;

- (void)setX:(float)xCoord;
- (void)setY:(float)yCoord;
- (void)setZ:(float)zCoord;

- (void)setFloat:(Vector3f *)vector;
- (void)setInt:(Vector3i *)vector;

- (BOOL)isNull;

- (void)add:(Vector3f *)addend;
- (void)addX:(float)xAddend y:(float)yAddend z:(float)zAddend;

- (void)sub:(Vector3f *)subtrahend;
- (void)subX:(float)xSubtrahend y:(float)ySubtrahend z:(float)zSubtrahend;

- (void)cross:(Vector3f *)m;
- (float)dot:(Vector3f *)m;
- (void)scale:(float)f;
- (void)normalize;
- (float)length;
- (float)lengthSquared;

- (NSComparisonResult)compareToVector:(Vector3f *)vector;
- (BOOL)isEqualToVector:(Vector3f *)vector;

@end
