//
//  BoundingBox.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface BoundingBox : NSObject {
    @private
    Vector3f* origin;
    Vector3f* dimensions;
}

- (id)initAtOrigin:(Vector3f *)theOrigin dimensions:(Vector3f *)theDimensions;
@end
