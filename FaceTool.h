//
//  FaceTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DefaultTool.h"

@class MapWindowController;
@class PickingHit;
@class Plane3D;
@class Vector3f;
@class DragFaceCursor;

@interface FaceTool : DefaultTool {
@private
    MapWindowController* windowController;
    Plane3D* plane;
    Vector3f* lastPoint;
    Vector3f* dragDir;
    DragFaceCursor* cursor;
}

- (id)initWithController:(MapWindowController *)theWindowController;

@end
