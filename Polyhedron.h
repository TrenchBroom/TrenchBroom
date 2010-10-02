//
//  Polyhedron.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Polyhedron : NSObject {
    NSMutableSet* sides;
}
- (id)initCuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions;
@end
