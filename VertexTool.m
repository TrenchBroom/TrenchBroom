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
#import "Face.h"

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
    NSAssert(event != NULL, @"event must not be NULL");
    NSAssert(ray != NULL, @"ray must not be NULL");
    NSAssert(hits != nil, @"hit list must not be nil");
    
    [hits retain];
    
    PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager setGroupsByEvent:NO];
        [undoManager beginUndoGrouping];
        
        id <Brush> brush = [hit object];
        [map snapBrushes:[NSArray arrayWithObject:brush]];
        
        int index = [hit vertexIndex];
        const TVertexList* vertices = [brush vertices];
        const TEdgeList* edges = [brush edges];
        
        if (index < vertices->count) {
            TVertex* vertex = vertices->items[index];
            lastPoint = vertex->vector;
        } else if (index < vertices->count + edges->count) {
            TEdge* edge = edges->items[index - vertices->count];
            centerOfEdge(edge, &lastPoint);
        } else {
            // the side index is not the same as the face index!!!
            id <Face> face = [[brush faces] objectAtIndex:index - edges->count - vertices->count];
            centerOfVertices([face vertices], &lastPoint);
        }
        editingPoint = lastPoint;
        
        NSMutableArray* mutableBrushes = [[NSMutableArray alloc] init];
        NSMutableArray* mutableVertexIndices = [[NSMutableArray alloc] init];
        
        NSEnumerator* hitEn = [[hits hitsOfType:HT_VERTEX] objectEnumerator];
        while ((hit = [hitEn nextObject])) {
            [mutableBrushes addObject:[hit object]];
            [mutableVertexIndices addObject:[NSNumber numberWithInt:[hit vertexIndex]]];
        }
        
        [map snapBrushes:mutableBrushes];
        
        brushes = mutableBrushes;
        vertexIndices = mutableVertexIndices;
        
        drag = YES;
    }
    [hits release];
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
    
    MapDocument* map = [windowController document];
    NSArray* newVertexIndices = [map dragVertices:vertexIndices brushes:brushes delta:&deltaf];
    
    NSEnumerator* indexEn = [newVertexIndices objectEnumerator];
    NSNumber* index;
    while ((index = [indexEn nextObject])) {
        if ([index intValue] == -1) {
            [self endLeftDrag:event ray:ray hits:hits];
            break;
        }
    }
    
    [vertexIndices release];
    vertexIndices = [newVertexIndices retain];
    lastPoint = point;
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager setActionName:[self actionName]];
    [undoManager endUndoGrouping];
    [undoManager setGroupsByEvent:YES];

    [brushes release];
    brushes = nil;
    [vertexIndices release];
    vertexIndices = nil;
    [editingSystem release];
    editingSystem = nil;
    
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
        NSArray* vertexHits = [hits hitsOfType:HT_VERTEX];
        if ([vertexHits count] == 0)
            return;

        position = *[[vertexHits objectAtIndex:0] hitPoint];
        
        BOOL attention = NO;
        NSEnumerator* hitEn = [vertexHits objectEnumerator];
        PickingHit* hit;
        while (!attention && (hit = [hitEn nextObject])) {
            id <Brush> brush = [hit object];
            const TVertexList* vertices = [brush vertices];
            for (int i = 0; i < vertices->count && !attention; i++)
                attention = !intV3f(&vertices->items[i]->vector);
        }
        
        [cursor setAttention:attention];
    } else {
        float dist = [editingSystem intersectWithRay:ray planePosition:&editingPoint];
        if (isnan(dist))
            return;
        
        rayPointAtDistance(ray, dist, &position);
        [cursor setAttention:NO];
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
