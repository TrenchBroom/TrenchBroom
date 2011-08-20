//
//  CreateBrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"

@protocol Brush;
@class MapWindowController;
@class BoundsFeedbackFigure;

@interface CreateBrushTool : DefaultTool {
    MapWindowController* windowController;
    TPlane plane;
    TBoundingBox initialBounds;
    TVector3f lastPoint;
    id <Brush> brush;
    BOOL drag;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
