//
//  FaceRotationTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceRotationTool.h"
#import "FaceRotationFigure.h"
#import "Plane3D.h"
#import "HalfSpace3D.h"
#import "Vector3f.h"
#import "Options.h"
#import "Face.h"
#import "Math.h"

static NSString* RotateCWKey = @"Rotate Texture Clockwise";
static NSString* RotateCCWKey = @"Rotate Texture Counterclockwise";

@implementation FaceRotationTool
- (void)updateDefaults:(NSDictionary *)defaults {
    rotateCWKey = [[defaults objectForKey:RotateCWKey] intValue];
    rotateCCWKey = [[defaults objectForKey:RotateCCWKey] intValue];
}

- (BOOL)face:(Face *)theFace hitByRay:(Ray3D *)theRay {
    Plane3D* plane = [[theFace halfSpace] boundary];
    Vector3f* is = [plane intersectWithRay:theRay];
    if (is == nil)
        return NO;
    
    [is sub:[theFace center]];
    float ls = [is lengthSquared];
    
    return fgte(ls, 144) && flte(ls, 256);
}

- (void)keyDown:(NSEvent *)theEvent {
    int keyCode = [theEvent keyCode];
    int d = 15;
    
    if (![options snapToGrid] ^ ([theEvent modifierFlags] & NSShiftKeyMask) != 0)
        d = 1;
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    if (keyCode == rotateCWKey)
        while ((face = [faceEn nextObject]))
            [face setRotation:[face rotation] + d];
    else if (keyCode == rotateCCWKey)
        while ((face = [faceEn nextObject]))
            [face setRotation:[face rotation] - d];
    else
        NSLog(@"unknown key code: %i", [theEvent keyCode]);
}

- (id)createFigure:(Face *)face {
    return [[[FaceRotationFigure alloc] initWithFace:face] autorelease];
}
@end
