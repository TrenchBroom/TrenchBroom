//
//  CompassView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <AppKit/AppKit.h>

@class Camera;
@class CompassFigure;

@interface CompassView : NSOpenGLView {
    Camera* camera;
    CompassFigure* compassFigure;
}

- (void)setCamera:(Camera *)theCamera;

@end
