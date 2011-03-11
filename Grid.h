//
//  Grid.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const GridChanged;

@class Vector3f;

@interface Grid : NSObject {
    @private
    int size;
    BOOL draw;
    BOOL snap;
}

- (int)size;
- (BOOL)draw;
- (BOOL)snap;

- (void)setSize:(int)theSize;
- (void)setDraw:(BOOL)isDrawEnabled;
- (void)setSnap:(BOOL)isSnapEnabled;
- (void)toggleDraw;
- (void)toggleSnap;

- (void)snapToGrid:(Vector3f *)vector;
- (Vector3f *)gridOffsetOf:(Vector3f *)vector;
@end
