/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"
#import "Math.h"

@class MapWindowController;
@class Picker;
@class PickingHitList;
@class Renderer;
@class Grid;
@class MapDocument;
@class ClipPlane;
@class ClipPointFeedbackFigure;
@class ClipLineFeedbackFigure;
@class ClipPlaneFeedbackFigure;
@class EditingPlaneFigure;

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
    NSMutableArray* brushFigures;
    TVector3i* currentPoint;
    ClipPointFeedbackFigure* currentFigure;
    int draggedPoint;
    EditingPlaneFigure* editingPlaneFigure;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)activate;
- (void)deactivate;
- (BOOL)active;

- (void)toggleClipMode;
- (NSArray *)performClip:(MapDocument* )map;
- (void)cancel;

- (void)deleteLastPoint;
- (int)numPoints;

@end
