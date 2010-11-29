//
//  Segment3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"

@interface Segment3D : NSObject {
    Vector3f* startVertex;
    Vector3f* endVertex;
}

- (id)initWithStartVertex:(Vector3f *)start endVertex:(Vector3f *)end;

- (Vector3f *)startVertex;
- (Vector3f *)endVertex;
@end
