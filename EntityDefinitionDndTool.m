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
#import "EntityDefinitionDndFeedbackFigure.h"
#import "Renderer.h"
#import "ControllerUtils.h"

@interface EntityDefinitionDndTool (private)

- (void)update:(id <NSDraggingInfo>)sender hits:(PickingHitList *)hits;

@end

@implementation EntityDefinitionDndTool (private)

- (void)update:(id <NSDraggingInfo>)sender hits:(PickingHitList *)hits {
    TVector3i posi;
    
    Grid* grid = [[windowController options] grid];
    calculateEntityOrigin([entity entityDefinition], hits, [sender draggingLocation], [windowController camera], &posi);
    [grid snapToGridV3i:&posi result:&posi];
    
    MapDocument* map = [windowController document];
    [map setEntity:entity propertyKey:OriginKey value:[NSString stringWithFormat:@"%i %i %i", posi.x, posi.y, posi.z]];
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

    NSUndoManager* undoManager = [map undoManager];
    [undoManager setGroupsByEvent:NO];
    [undoManager beginUndoGrouping];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager removeAll:YES];
    [selectionManager addEntity:entity record:YES];

    [self update:sender hits:hits];
    return NSDragOperationCopy;
}

- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    [self update:sender hits:hits];
    return NSDragOperationCopy;
}

- (void)handleDraggingEnded:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (entity != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager endUndoGrouping];
        [undoManager undo];
        [undoManager setGroupsByEvent:YES];
        
        entity = nil;
    }
}

- (void)handleDraggingExited:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (entity != nil) {
        MapDocument* map = [windowController document];
        NSUndoManager* undoManager = [map undoManager];
        [undoManager endUndoGrouping];
        [undoManager undo];
        [undoManager setGroupsByEvent:YES];
        
        entity = nil;
    }
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    return YES;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits {
    MapDocument* map = [windowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Create Entity"];
    [undoManager setGroupsByEvent:YES];
    
    entity = nil;
}


@end
