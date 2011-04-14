//
//  Cursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@protocol Cursor <NSObject>

- (void)render;
- (void)update:(Vector3f *)thePosition;

@end
