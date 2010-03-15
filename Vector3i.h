//
//  Vector3i.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Vector3i : NSObject {
	int x;
	int y;
	int z;
}

- (id)initWithVector:(Vector3i *)vector;
- (id)initWithXCoord:(int)xCoord yCoord:(int)yCoord zCoord:(int) zCoord;

- (int)x;
- (int)y;
- (int)z;

- (void)setX:(int)xCoord;
- (void)setY:(int)yCoord;
- (void)setZ:(int)zCoord;

- (void)set:(Vector3i *)vector;
@end
