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
@class BrushToolCursor;

@interface BrushTool : DefaultTool {
    @private
    MapWindowController* windowController;
    TPlane plane;
    TVector3f lastPoint;
    BrushToolCursor* cursor;
    BOOL drag;
}

- (id)initWithController:(MapWindowController *)theWindowController;

@end
