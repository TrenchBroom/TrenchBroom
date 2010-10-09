//
//  HalfSpace3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Plane3D.h"

@interface HalfSpace3D : NSObject {

}

- (BOOL)contains:(Vector3f *)point;
- (Plane3D *)boundary;
@end
