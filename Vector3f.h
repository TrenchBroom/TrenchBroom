//
//  Vector3f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Vector3f : NSObject {
	float x;
	float y;
	float z;
}
- (id)initWithVector:(Vector3f *)vector;
- (id)initWithXCoord:(float)xCoord yCoord:(float)yCoord zCoord:(float) zCoord;

- (float)x;
- (float)y;
- (float)z;

- (void)setX:(float)xCoord;
- (void)setY:(float)yCoord;
- (void)setZ:(float)zCoord;

- (void)set:(Vector3f *)vector;
@end
