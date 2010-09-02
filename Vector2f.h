//
//  Vector2f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Vector2f : NSObject {
	float x;
	float y;
}
+ (Vector2f *)add:(Vector2f *)left addend:(Vector2f *)right;
+ (Vector2f *)sub:(Vector2f *)left subtrahend:(Vector2f *)right;
+ (Vector2f *)normalize:(Vector2f *)vector;

- (id)initWithVector:(Vector2f *)vector;
- (id)initWithX:(float)xCoord y:(float)yCoord;

- (float)x;
- (float)y;

- (void)setX:(float)xCoord;
- (void)setY:(float)yCoord;

- (void)set:(Vector2f *)vector;

- (BOOL)isNull;

- (void)add:(Vector2f *)addend;
- (void)addX:(float)xAddend y:(float)yAddend;

- (void)sub:(Vector2f *)subtrahend;
- (void)subX:(float)xSubtrahend y:(float)ySubtrahend;

- (float)dot:(Vector2f *)m;
- (void)normalize;
- (float)length;
- (float)lengthSquared;

- (NSComparisonResult)lexicographicCompare:(Vector2f *)vector;
- (BOOL)smallerThan:(Vector2f *)vector;

@end
