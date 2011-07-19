//
//  EntityDefinitionDragTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionDndTool.h"
#import "EntityDefinition.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Camera.h"
#import "SelectionManager.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "Face.h"
#import "Grid.h"
#import "Options.h"

@interface EntityDefinitionDndTool (private)

- (TVector3i)entityPosition:(id <NSDraggingInfo>)sender hits:(PickingHitList *)hits;

@end

@implementation EntityDefinitionDndTool (private)

- (TVector3i)entityPosition:(id <NSDraggingInfo>)sender hits:(PickingHitList *)hits {
    TVector3i posi;
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    Grid* grid = [[windowController options] grid];

    if (hit != nil) {
        const TVector3f* hitPoint = [hit hitPoint];
        TVector3f size;
        sizeOfBounds([entity bounds], &size);
        
        id <Face> face = [hit object];
        TVector3f* faceNorm = [face norm];
        if (faceNorm->x >= 0)
            posi.x = hitPoint->x;
        else
            posi.x = hitPoint->x - size.x;
        if (faceNorm->y >= 0)
            posi.y = hitPoint->y;
        else
            posi.y = hitPoint->y - size.y;
        if (faceNorm->z >= 0)
            posi.z = hitPoint->z;
        else
            posi.z = hitPoint->z - size.z;
        [grid snapToGridV3i:&posi direction:faceNorm result:&posi];
    } else {
        Camera* camera = [windowController camera];
        NSPoint location = [sender draggingLocation];
        TVector3f posf = [camera unprojectX:location.x y:location.y depth:0.94f];
        roundV3f(&posf, &posi);
        [grid snapToGridV3i:&posi result:&posi];
    }
    
    return posi;
}

@end

@implementation EntityDefinitionDndTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    NSAssert(theWindowController != nil, @"window controller must not be nil");
    
    if ((self = [self init])) {
        windowController = theWindowController;
    }
    
    return self;
}

- (NSDragOperation)handleDraggingEntered:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSString* type = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:EntityDefinitionType]];

    MapDocument* map = [windowController document];
    NSString* className = [[NSString alloc] initWithData:[pasteboard dataForType:type] encoding:NSUTF8StringEncoding];
    entity = [map createEntityWithClassname:className];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager removeAll:YES];
    [selectionManager addEntity:entity record:YES];

    TVector3i delta;
    TVector3i position = [self entityPosition:sender hits:hits];
    TVector3i* origin = [entity origin];
    subV3i(&position, origin, &delta);
    
    [map translateEntities:[NSSet setWithObject:entity] delta:delta];
    
    return NSDragOperationCopy;
}

- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    TVector3i delta;
    TVector3i position = [self entityPosition:sender hits:hits];
    TVector3i* origin = [entity origin];
    subV3i(&position, origin, &delta);
    
    MapDocument* map = [windowController document];
    [map translateEntities:[NSSet setWithObject:entity] delta:delta];
    return NSDragOperationCopy;
}

- (void)handleDraggingEnded:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
}

- (void)handleDraggingExited:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    return YES;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
}


@end
