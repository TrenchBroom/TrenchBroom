//
//  RotateTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"

@class MapWindowController;
@class RotateCursor;
@class RotationFeedbackFigure;

@interface RotateTool : DefaultTool {
@private
    MapWindowController* windowController;
    RotateCursor* rotateCursor;
    RotationFeedbackFigure* feedbackFigure;
    BOOL drag;
    TVector3f center;
    TVector3f initialVector;
    float radius;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;


@end
