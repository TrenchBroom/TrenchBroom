//
//  VertexTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "VertexTool.h"
#import "Math.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "CursorManager.h"
#import "DragVertexCursor.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "EditingSystem.h"
#import "Options.h"
#import "Grid.h"
#import "SelectionManager.h"
#import "MutableBrush.h"

@interface VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return keyStatus == KS_OPTION;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (editingSystem != nil)
        [editingSystem release];

    Camera* camera = [windowController camera];
    editingSystem = [[EditingSystem alloc] initWithCamera:camera vertical:[self isAlternatePlaneModifierPressed]];
}

@end

@implementation VertexTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        cursor = [[DragVertexCursor alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [cursor release];
    [editingSystem release];
    [super dealloc];
}

- (void)handleKeyStatusChanged:(NSEvent *)event status:(EKeyStatus)theKeyStatus ray:(TRay *)ray hits:(PickingHitList *)hits {
    keyStatus = theKeyStatus;
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit == nil)
        return;
    
    vertexHits = [[hits hitsOfType:HT_VERTEX] retain];
    
    id <Brush> brush = [hit object];
    TVertex* vertex = [brush vertices]->items[[hit vertexIndex]];
    
    lastPoint = vertex->vector;
    editingPoint = lastPoint;
    drag = YES;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    Options* options = [windowController options];
    Grid* grid = [options grid];
    [grid snapToGridV3f:&point result:&point];

    TVector3f deltaf;
    subV3f(&point, &lastPoint, &deltaf);

    if (nullV3f(&deltaf))
        return;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    NSMutableArray* vertexIndices = [[NSMutableArray alloc] init];

    NSEnumerator* hitEn = [vertexHits objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject])) {
        MutableBrush* brush = [hit object];
        if ([selectionManager isBrushSelected:brush]) {
            [vertexIndices addObject:[NSNumber numberWithInt:[hit vertexIndex]]];
            [brushes addObject:brush];
        }
    }
    
    MapDocument* map = [windowController document];
    if (![map dragVertices:vertexIndices brushes:brushes delta:&deltaf]) {
        [vertexHits release];
        vertexHits = nil;
        drag = NO;
    }
    
    [brushes release];
    [vertexIndices release];
    
    lastPoint = point;
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    [vertexHits release];
    vertexHits = nil;
    drag = NO;
}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager pushCursor:cursor];
    [self updateCursor:event ray:ray hits:hits];
}

- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager popCursor];
}

- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    TVector3f position;

    if (!drag) {
        PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
        if (hit == nil)
            return;
        
        position = *[hit hitPoint];
    } else {
        float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
        if (isnan(dist))
            return;
        
        rayPointAtDistance(ray, dist, &position);
    }
    
    [self updateMoveDirectionWithRay:ray hits:hits];

    Camera* camera = [windowController camera];
    
    [cursor setEditingSystem:editingSystem];
    [cursor setPosition:&position];
    [cursor setCameraPosition:[camera position]];
}

- (NSString *)actionName {
    return @"Drag Vertices";
}
@end
