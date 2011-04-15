//
//  ClipTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"

@class MapWindowController;
@class Vector3i;
@class Picker;
@class PickingHitList;
@class Ray3D;
@class Renderer;
@class Grid;
@class MapDocument;
@class ClipPlane;
@class ClipPointFeedbackFigure;
@class ClipLineFeedbackFigure;
@class ClipPlaneFeedbackFigure;

@interface ClipTool : DefaultTool {
    MapWindowController* windowController;
    ClipPlane* clipPlane;
    ClipPointFeedbackFigure* point1Figure;
    ClipPointFeedbackFigure* point2Figure;
    ClipPointFeedbackFigure* point3Figure;
    ClipLineFeedbackFigure* line1Figure;
    ClipLineFeedbackFigure* line2Figure;
    ClipLineFeedbackFigure* line3Figure;
    ClipPlaneFeedbackFigure* planeFigure;
    NSMutableSet* brushFigures;
    Vector3i* currentPoint;
    ClipPointFeedbackFigure* currentFigure;
    Vector3i* draggedPoint;
    BOOL active;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)activate;
- (void)deactivate;
- (BOOL)active;

- (void)toggleClipMode;
- (NSSet *)performClip:(MapDocument* )map;
- (void)cancel;

- (void)deleteLastPoint;
- (int)numPoints;

@end
