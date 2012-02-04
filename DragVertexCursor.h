//
//  DragVertexCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "DefaultTool.h"
#import "Cursor.h"
#import "Math.h"

@class DoubleArrowFigure;
@class EditingSystem;

@interface DragVertexCursor : NSObject <Cursor> {
    DoubleArrowFigure* arrows[3];
    EditingSystem* editingSystem;
    TVector3f position;
    TVector3f cameraPosition;
    BOOL attention;
}

- (void)setEditingSystem:(EditingSystem *)theEditingSystem;
- (void)setCameraPosition:(const TVector3f *)theCameraPosition;
- (void)setAttention:(BOOL)theAttention;

@end
