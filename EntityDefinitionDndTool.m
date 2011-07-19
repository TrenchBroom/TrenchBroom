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
    
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TVector3f posf = [camera unprojectX:location.x y:location.y depth:0.94f];
    
    TVector3i posi, delta;
    roundV3f(&posf, &posi);
    
    TVector3i* origin = [entity origin];
    subV3i(&posi, origin, &delta);
    
    [map translateEntities:[NSSet setWithObject:entity] delta:delta];
    
    return NSDragOperationCopy;
}

- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    NSPoint location = [sender draggingLocation];
    TVector3f posf = [camera unprojectX:location.x y:location.y depth:0.94f];
    
    TVector3i posi, delta;
    roundV3f(&posf, &posi);
    
    TVector3i* origin = [entity origin];
    subV3i(&posi, origin, &delta);
    
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
