//
//  BrushTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"

@class MapWindowController;
@class Plane3D;
@class Vector3f;
@class BrushToolCursor;

@interface BrushTool : DefaultTool {
    @private
    MapWindowController* windowController;
    Plane3D* plane;
    Vector3f* lastPoint;
    BrushToolCursor* cursor;
}

- (id)initWithController:(MapWindowController *)theWindowController;

@end
