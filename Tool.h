//
//  Tool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Ray3D;

@protocol Tool <NSObject>

- (void)translateTo:(Ray3D *)theRay toggleSnap:(BOOL)toggleSnap altPlane:(BOOL)altPlane;
- (NSString *)actionName;
@end
