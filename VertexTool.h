//
//  VertexTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "DefaultTool.h"

@class MapWindowController;
@class DragVertexCursor;
@class EditingSystem;
@protocol Brush;

typedef enum {
    VTS_DEFAULT,
    VTS_DRAG,
    VTS_CANCEL
} EVertexToolState;

@interface VertexTool : DefaultTool {
    MapWindowController* windowController;
    DragVertexCursor* cursor;
    EditingSystem* editingSystem;
    EVertexToolState state;
    id <Brush> brush;
    int index;
    TVector3f lastPoint;
    TVector3f editingPoint;
    EKeyStatus keyStatus;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
