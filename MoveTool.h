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

typedef enum {
    MD_LR_FB, // left / right & front / back
    MD_LR_UD // left / right & up / down
} EMoveDirection;

@class MapWindowController;
@class EditingSystem;
@class MoveToolFeedbackFigure;

@interface MoveTool : DefaultTool {
    @private
    MapWindowController* windowController;
    EditingSystem* editingSystem;
    TVector3f editingPoint;
    MoveToolFeedbackFigure* feedbackFigure;
    TVector3f lastPoint;
    BOOL drag;
    EMoveDirection moveDirection;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
