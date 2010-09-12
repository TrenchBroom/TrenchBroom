//
//  Math.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern float const AlmostZero;

@interface Math : NSObject {

}

+ (BOOL)is:(float)v betEx:(float)b1 andEx:(float)b2;
+ (BOOL)is:(float)v betEx:(float)b1 andIn:(float)b2;
+ (BOOL)is:(float)v betIn:(float)b1 andEx:(float)b2;
+ (BOOL)is:(float)v betIn:(float)b1 andIn:(float)b2;
+ (BOOL)is:(float)v1 eq:(float)v2;
+ (BOOL)is:(float)v1 gt:(float)v2;
+ (BOOL)is:(float)v1 lt:(float)v2;
+ (BOOL)is:(float)v1 gte:(float)v2;
+ (BOOL)is:(float)v1 lte:(float)v2;
+ (BOOL)pos:(float)v;
+ (BOOL)neg:(float)v;
+ (BOOL)zer:(float)v;
@end
