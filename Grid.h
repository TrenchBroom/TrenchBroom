/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import "Math.h"

extern NSString* const GridChanged;

extern int const GridMaxSize;
extern int const GridMinSize;

@protocol Face;

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
- (float)actualRotAngle;
- (BOOL)draw;
- (BOOL)snap;

- (void)setSize:(int)theSize;
- (void)setAlpha:(float)theAlpha;
- (void)setDraw:(BOOL)isDrawEnabled;
- (void)setSnap:(BOOL)isSnapEnabled;
- (void)toggleDraw;
- (void)toggleSnap;

- (float)snapToGridf:(float)f;
- (float)snapUpToGridf:(float)f;
- (float)snapDownToGridf:(float)f;
- (float)snapAngle:(float)a;

- (void)snapToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)snapToFarthestGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)snapUpToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)snapDownToGridV3f:(const TVector3f *)vector result:(TVector3f *)result;
- (void)gridOffsetV3f:(const TVector3f *)vector result:(TVector3f *)result;

- (void)snapToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapUpToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapDownToGridV3i:(const TVector3i *)vector result:(TVector3i *)result;
- (void)snapToGridV3i:(const TVector3i *)vector direction:(TVector3f *)direction result:(TVector3i *)result;
- (void)gridOffsetV3i:(const TVector3i *)vector result:(TVector3i *)result;

- (float)intersectWithRay:(const TRay *)ray skip:(int)skip;

- (void)moveDeltaForBounds:(const TBoundingBox *)theBounds worldBounds:(const TBoundingBox *)theWorldBounds delta:(TVector3f *)theDelta lastPoint:(TVector3f *)theLastPoint;
- (void)moveDeltaForVertex:(const TVector3f *)theVertex worldBounds:(const TBoundingBox *)theWorldBounds delta:(TVector3f *)theDelta lastPoint:(TVector3f *)theLastPoint;
- (float)dragDeltaForFace:(id <Face>)theFace delta:(TVector3f *)theDelta;

- (void)activateTexture;
- (void)deactivateTexture;
@end
