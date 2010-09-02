//
//  Line2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Line2D.h"
#import "Vector2f.h"

@interface Line2D : NSObject {
    Vector2f* point;
    Vector2f* direction; // normalized
}

- (id)initWithPoint1:(Vector2f *)point1 point2:(Vector2f *)point2;
- (id)initWithPoint:(Vector2f *)p normalizedDirection:(Vector2f *)d;
- (id)initWithPoint:(Vector2f *)p direction:(Vector2f *)d;

- (Vector2f *)point;
- (Vector2f *)direction;

- (float)yAt:(float)x;

- (Vector2f *)intersectWith:(Line2D *)line;
- (BOOL)isParallelTo:(Line2D *)line;

@end
