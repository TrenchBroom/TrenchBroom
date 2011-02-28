//
//  Vector3i.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface Vector3i : NSObject {
    @private
    int coords[3];
}

+ (Vector3i *)nullVector;

- (id)initWithVector:(Vector3i *)vector;
- (id)initWithX:(int)xCoord y:(int)yCoord z:(int) zCoord;

- (int)x;
- (int)y;
- (int)z;

- (void)setX:(int)xCoord;
- (void)setY:(int)yCoord;
- (void)setZ:(int)zCoord;

- (void)set:(Vector3i *)vector;

- (BOOL)null;

- (void)add:(Vector3i *)addend;
- (void)addX:(int)xAddend Y:(int)yAddend Z:(int)zAddend;

- (void)cross:(Vector3i *)m;
- (void)scale:(float)f;

- (NSComparisonResult)compareToVector:(Vector3i *)vector;
- (BOOL)isEqualToVector:(Vector3i *)vector;
@end
