//
//  Segment3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Segment3D : NSObject {
    @private
    Vector3f* startVertex;
    Vector3f* endVertex;
}

+ (Segment3D *)segmentWithStartVertex:(Vector3f *)start endVertex:(Vector3f *)end;

- (id)initWithStartVertex:(Vector3f *)start endVertex:(Vector3f *)end;

- (Vector3f *)startVertex;
- (Vector3f *)endVertex;
- (void)flip;
@end
