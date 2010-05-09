//
//  RenderFace.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Face.h"

@interface RenderPolygon : NSObject {
    NSMutableArray* points;
}

-(id)initWithFace:(Face*)face;

@end
