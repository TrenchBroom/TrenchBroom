//
//  FaceRotationFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Face;
@class RenderContext;

@interface FaceRotationFigure : NSObject {
    @private
    Face* face;
}

- (id)initWithFace:(Face *)theFace;

- (void)render:(RenderContext *)renderContext;

@end
