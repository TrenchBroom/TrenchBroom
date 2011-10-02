/*
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

#import "MoveTool.h"
#import "Grid.h"
#import "Options.h"
#import "Camera.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "SelectionManager.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Face.h"
#import "Brush.h"
#import "Entity.h"
#import "MutableBrush.h"
#import "math.h"
#import "Renderer.h"
#import "MapView3D.h"
#import "CursorManager.h"
#import "MoveCursor.h"
#import "RotateCursor.h"
#import "ControllerUtils.h"
#import "EditingPlane.h"

@interface MoveTool (private)

- (BOOL)isDuplicateModifierPressed;

- (BOOL)beginMove:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)move:(const TVector3i *)delta;
- (void)endMove:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

@end

@implementation MoveTool (private)

- (BOOL)isDuplicateModifierPressed {
    return [NSEvent modifierFlags] == NSCommandKeyMask;
}

- (BOOL)beginMove:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:YES];
    if (hit == nil)
        return NO;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    Camera* camera = [windowController camera];
    if ([hit type] == HT_FACE) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        
        if (![selectionManager isBrushSelected:brush])
            return NO;
    } else {
        id <Entity> entity = [hit object];
        
        if (![selectionManager isEntitySelected:entity])
            return NO;
    }

    lastPoint = *[hit hitPoint];
    editingPlane = [[camera editingPlane] retain];
    editingPlanePoint = lastPoint;
    
    duplicate = [self isDuplicateModifierPressed];
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    return YES;
}

- (void)move:(const TVector3i *)delta {
    Options* options = [windowController options];
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    
    if (duplicate) {
        NSMutableArray* newEntities = [[NSMutableArray alloc] init];
        NSMutableArray* newBrushes = [[NSMutableArray alloc] init];
        [map duplicateEntities:[selectionManager selectedEntities] newEntities:newEntities newBrushes:newBrushes];
        [map duplicateBrushes:[selectionManager selectedBrushes] newBrushes:newBrushes];
        
        [selectionManager removeAll:YES];
        [selectionManager addEntities:newEntities record:YES];
        [selectionManager addBrushes:newBrushes record:YES];
        
        [newEntities release];
        [newBrushes release];
        
        duplicate = NO;
    }
    
    [map translateBrushes:[selectionManager selectedBrushes] delta:*delta lockTextures:[options lockTextures]];
    [map translateEntities:[selectionManager selectedEntities] delta:*delta];
}

- (void)endMove:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];
    
    [editingPlane release];
    editingPlane = nil;
}

@end

@implementation MoveTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        moveCursor = [[MoveCursor alloc] init];
    }
    return self;
}

- (void)dealloc {
    [moveCursor release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Tool

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!scroll)
        drag = [self beginMove:event ray:ray hits:hits];
    else
        drag = YES;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = [editingPlane intersectWithRay:ray planePosition:&editingPlanePoint];
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    TVector3f deltaf;
    subV3f(&point, &lastPoint, &deltaf);
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    
    MapDocument* map = [windowController document];
    TBoundingBox* worldBounds = [map worldBounds];

    TBoundingBox bounds;
    SelectionManager* selectionManager = [map selectionManager];
    [selectionManager selectionBounds:&bounds];

    calculateMoveDelta(grid, &bounds, worldBounds, &deltaf, &lastPoint);
    if (nullV3f(&deltaf))
        return;
    
    TVector3i deltai;
    roundV3f(&deltaf, &deltai);
    
    [self move:&deltai];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    if (!scroll)
        [self endMove:event ray:ray hits:hits];
    drag = NO;
}

- (void)beginLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        scroll = [self beginMove:event ray:ray hits:hits];
    else
        scroll = YES;
}

- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!scroll)
        return;
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    
    TVector3f deltaf;
    TVector3i deltai;
    
    float d = ([event deltaY] > 0 ? 1 : -1) * [grid actualSize];
    
    scaleV3f([editingPlane backAxis], d, &deltaf);
    roundV3f(&deltaf, &deltai);
    
    [self move:&deltai];
    
    addV3f(&editingPlanePoint, &deltaf, &editingPlanePoint);
}

- (void)endLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!scroll)
        return;
    
    if (!drag)
        [self endMove:event ray:ray hits:hits];
    scroll = NO;
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:moveCursor];
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag && !scroll) {
        PickingHit* hit = [hits firstHitOfType:HT_FACE | HT_ENTITY ignoreOccluders:YES];
        if (hit != nil) {
            Camera* camera = [windowController camera];
            [moveCursor setPlaneNormal:strongestComponentV3f([[camera editingPlane] frontAxis])];
            [moveCursor update:[hit hitPoint]];
        }
    } else {
        float dist = [editingPlane intersectWithRay:ray planePosition:&editingPlanePoint];
        TVector3f position;
        rayPointAtDistance(ray, dist, &position);
        [moveCursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
