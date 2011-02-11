//
//  Tool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 11.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Ray3D.h"

@protocol Tool

- (void)startDrag:(Ray3D *)theRay;
- (void)drag:(Ray3D *)theRay;
- (void)endDrag:(Ray3D *)theRay;

- (BOOL)hitByRay:(Ray3D *)theRay;
@end
