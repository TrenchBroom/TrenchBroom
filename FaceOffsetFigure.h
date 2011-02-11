//
//  FaceOffsetFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 11.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@class Face;

@interface FaceOffsetFigure : NSObject {
    @private
    Face* face;
}

- (id)initWithFace:(Face *)theFace;

- (void)render:(RenderContext *)renderContext;

@end
