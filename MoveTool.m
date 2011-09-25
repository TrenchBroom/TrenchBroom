//
//  BrushTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
        
        lastPoint = *[hit hitPoint];
        
        plane.point = lastPoint;
        plane.norm = *closestAxisV3f([camera direction]);
    } else {
        id <Entity> entity = [hit object];
        
        if (![selectionManager isEntitySelected:entity])
            return NO;
        
        lastPoint = *[hit hitPoint];
        
        plane.point = lastPoint;
        plane.norm = *closestAxisV3f([camera direction]);
    }
    
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
    
    float dist = intersectPlaneWithRay(&plane, ray);
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
    
    scaleV3f(&plane.norm, d, &deltaf);
    roundV3f(&deltaf, &deltai);
    
    [self move:&deltai];
    
    addV3f(&plane.point, &deltaf, &plane.point);

    float dist = intersectPlaneWithRay(&plane, ray);
    if (isnan(dist))
        return;
    
    rayPointAtDistance(ray, dist, &lastPoint);
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
    if (!drag) {
        PickingHit* hit = [hits firstHitOfType:HT_FACE | HT_ENTITY ignoreOccluders:YES];
        if (hit != nil) {
            Camera* camera = [windowController camera];
            switch ([hit type]) {
                case HT_ENTITY: {
                    [moveCursor setPlaneNormal:strongestComponentV3f(closestAxisV3f([camera direction]))];
                    break;
                }
                case HT_FACE: {
                    [moveCursor setPlaneNormal:strongestComponentV3f(closestAxisV3f([camera direction]))];
                    break;
                }
                default:
                    break;
            }
            [moveCursor update:[hit hitPoint]];
        }
    } else {
        float dist = intersectPlaneWithRay(&plane, ray);
        TVector3f position;
        rayPointAtDistance(ray, dist, &position);
        [moveCursor update:&position];
    }
}

- (NSString *)actionName {
    return @"Move Objects";
}

@end
