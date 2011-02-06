//
//  Math.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern float const AlmostZero;

BOOL fzero(float v);
BOOL fpos(float v);
BOOL fneg(float v);
BOOL feq(float v1, float v2);
BOOL fgt(float v1, float v2);
BOOL flt(float v1, float v2);
BOOL fgte(float v1, float v2);
BOOL flte(float v1, float v2);
BOOL finxx(float v, float b1, float b2);
BOOL finxi(float v, float b1, float b2);
BOOL finix(float v, float b1, float b2);
BOOL finii(float v, float b1, float b2);

NSArray* makeCircle(float radius, int segments);
NSArray* makeRing(float innerRadius, float outerRadius, int segments);
