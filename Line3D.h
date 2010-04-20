//
//  Line.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h";

@interface Line3D : NSObject {
    Vector3f* p;
    Vector3f* d; // normalized
}

- (id)initWithPoint:(Vector3f *)point normalizedDirection:(Vector3f *)dir;
- (id)initWithPoint:(Vector3f *)point direction:(Vector3f *)dir;

- (Vector3f *)p;
- (Vector3f *)d;

- (Vector3f *)intersectionWithLine:(Line3D *)line;

@end
