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
@class RotateFeedbackFigure;

@interface RotateTool : DefaultTool {
@private
    MapWindowController* windowController;
    RotateCursor* rotateCursor;
    RotateFeedbackFigure* feedbackFigure;
    BOOL drag;
    TVector3f center;
    EAxis vAxis;
    float radius;
    NSPoint delta;
    float initialHAngle;
    float initialVAngle;
    float lastHAngle;
    float lastVAngle;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;


@end
