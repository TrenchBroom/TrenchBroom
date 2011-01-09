//
//  Line2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector2f;

@interface Line2D : NSObject {
@private
    Vector2f* point;
    Vector2f* direction; // normalized
}
+ (Line2D *)lineWithPoint1:(Vector2f *)point1 point2:(Vector2f *)point2;
+ (Line2D *)lineWithPoint:(Vector2f *)p normalizedDirection:(Vector2f *)d;
+ (Line2D *)lineWithPoint:(Vector2f *)p direction:(Vector2f *)d;
+ (Line2D *)lineWithLine:(Line2D *)l;

- (id)initWithPoint1:(Vector2f *)point1 point2:(Vector2f *)point2;
- (id)initWithPoint:(Vector2f *)p normalizedDirection:(Vector2f *)d;
- (id)initWithPoint:(Vector2f *)p direction:(Vector2f *)d;
- (id)initWithLine:(Line2D *)l;

- (Vector2f *)point;
- (Vector2f *)direction;

- (BOOL)isHorizontal;
- (BOOL)isVertical;

- (float)yAt:(float)x;

- (Vector2f *)intersectWith:(Line2D *)line;
- (BOOL)isParallelTo:(Line2D *)line;

@end
