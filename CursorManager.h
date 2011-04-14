//
//  CursorManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const CursorChanged;

@class Vector3f;
@protocol Cursor;

@interface CursorManager : NSObject {
    NSMutableArray* cursorStack;
}

- (void)pushCursor:(id <Cursor>)cursor;
- (void)popCursor;
- (void)updateCursor:(Vector3f *)thePosition;

- (BOOL)empty;
- (void)render;

@end
