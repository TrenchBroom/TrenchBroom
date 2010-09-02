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

+ (BOOL)is:(float)v betweenBoundary:(float)b1 andBoundary:(float)b2;

@end
