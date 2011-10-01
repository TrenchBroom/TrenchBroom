//
//  BrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"
#import "Math.h"

@class MapWindowController;
@class EditingPlane;
@class MoveCursor;

@interface MoveTool : DefaultTool {
    @private
    MapWindowController* windowController;
    EditingPlane* editingPlane;
    TVector3f editingPlanePoint;
    TVector3f lastPoint;
    MoveCursor* moveCursor;
    BOOL drag;
    BOOL scroll;
    BOOL duplicate;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
