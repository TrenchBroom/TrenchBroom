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
    float alpha;
    BOOL draw;
    BOOL snap;
    GLuint texIds[9];
    BOOL valid[9];
}

- (int)size;
- (float)alpha;
- (int)actualSize;
- (BOOL)draw;
- (BOOL)snap;

- (void)setSize:(int)theSize;
- (void)setAlpha:(float)theAlpha;
- (void)setDraw:(BOOL)isDrawEnabled;
- (void)setSnap:(BOOL)isSnapEnabled;
- (void)toggleDraw;
- (void)toggleSnap;

- (void)snapToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)snapUpToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)snapDownToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)gridOffsetV3f:(const TVector3f *)vector result:(TVector3f *)result;

- (void)snapToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapUpToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapDownToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapToGridV3i:(const TVector3i *)vector direction:(TVector3f *)direction result:(TVector3i *)result;
- (void)gridOffsetV3i:(const TVector3i *)vector result:(TVector3i *)result;

- (void)activateTexture;
- (void)deactivateTexture;
@end
