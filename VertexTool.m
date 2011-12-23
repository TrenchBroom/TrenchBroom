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
#import "CursorManager.h"
#import "DragVertexCursor.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "EditingSystem.h"

@interface VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation VertexTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    EditingSystem* newEditingSystem;
    const TVector3f* norm;
    
    Camera* camera = [windowController camera];
    
    PickingHit* hit = [theHits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
    if (hit == nil)
        return;
    
    norm = oppositeAxisV3f([camera direction]);
    newEditingSystem = [[EditingSystem alloc] initWithCamera:camera yAxis:norm invert:[self isAlternatePlaneModifierPressed]];
    
    if (newEditingSystem != nil) {
        if (editingSystem != nil)
            [editingSystem release];
        editingSystem = newEditingSystem;
    }
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
        PickingHit* hit = [hits firstHitOfType:HT_VERTEX ignoreOccluders:NO];
        if (hit == nil)
            return;
        
        position = *[hit hitPoint];
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
