//
//  Grid.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import "Math.h"

extern NSString* const GridChanged;

@interface Grid : NSObject {
    @private
    int size;
    BOOL draw;
    BOOL snap;
    GLuint texIds[9];
}

- (int)size;
- (int)actualSize;
- (BOOL)draw;
- (BOOL)snap;

- (void)setSize:(int)theSize;
- (void)setDraw:(BOOL)isDrawEnabled;
- (void)setSnap:(BOOL)isSnapEnabled;
- (void)toggleDraw;
- (void)toggleSnap;

- (void)snapToGrid:(TVector3f *)vector result:(TVector3f *)result;
- (void)snapUpToGrid:(TVector3f *)vector result:(TVector3f *)result;
- (void)snapDownToGrid:(TVector3f *)vector result:(TVector3f *)result;
- (void)gridOffsetOf:(TVector3f *)vector result:(TVector3f *)result;

- (void)activateTexture;
- (void)deactivateTexture;
@end
