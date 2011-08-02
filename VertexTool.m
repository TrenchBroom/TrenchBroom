//
//  VertexTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "VertexTool.h"
#import "MapWindowController.h"
#import "CursorManager.h"
#import "Math.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Vertex.h"

@implementation VertexTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
        vertexHits = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [vertexHits release];
    [super dealloc];
}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    PickingHit* firstHit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:YES];
    NSAssert(firstHit != nil, @"must have a vertex hit to initiate vertex dragging");

    Vertex* firstVertex = [firstHit object];
    TVector3f* firstVector = [firstVertex vector];
    
    NSArray* otherHits = [hits hitsOfType:HT_VERTEX];
    NSEnumerator* hitEn = [otherHits objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject])) {
        Vertex* vertex = [hit object];
        if (equalV3f(firstVector, [vertex vector]))
            [vertexHits addObject:hit];
    }
    
    NSLog(@"vertex drag started with %lu vertices", [vertexHits count]);
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSLog(@"vertex dragged");
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSLog(@"vertex drag ended with %lu vertices", [vertexHits count]);
    [vertexHits removeAllObjects];
}

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSLog(@"vertex tool activated");
}

- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSLog(@"vertex tool deactivated");
}

- (NSString *)actionName {
    return @"Drag Vertices";
}

@end
