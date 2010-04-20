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

- (id)initWithVector:(Vector2f *)vector;
- (id)initWithXCoord:(float)xCoord yCoord:(float)yCoord;

- (float)x;
- (float)y;

- (void)setX:(float)xCoord;
- (void)setY:(float)yCoord;

- (void)set:(Vector2f *)vector;

- (BOOL)null;

- (void)add:(Vector2f *)addend;
- (void)addX:(float)xAddend Y:(float)yAddend;

- (void)sub:(Vector2f *)subtrahend;
- (void)subX:(float)xSubtrahend Y:(float)ySubtrahend;

- (float)dot:(Vector2f *)m;
- (void)normalize;
- (float)length;
- (float)lengthSquared;

@end
