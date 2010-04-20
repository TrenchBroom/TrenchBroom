//
//  Line2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 20.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector2f.h"

@interface Line2D : NSObject {
    Vector2f* p;
    Vector2f* d; // normalized
}

- (id)initWithPoint:(Vector2f *)point normalizedDirection:(Vector2f *)dir;
- (id)initWithPoint:(Vector2f *)point direction:(Vector2f *)dir;

- (Vector2f *)p;
- (Vector2f *)d;

- (Vector2f *)intersectionWithLine:(Line2D *)line;
@end
