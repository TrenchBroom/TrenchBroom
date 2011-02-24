//
//  ToolManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ToolManager.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "Options.h"
#import "Tool.h"
#import "FaceOffsetTool.h"
#import "FaceRotationTool.h"
#import "Face.h"

NSString* const FiguresAdded = @"FiguresAdded";
NSString* const FiguresRemoved = @"FiguresRemoved";
NSString* const FiguresKey = @"Figures";

@implementation ToolManager

- (id)init {
    if (self = [super init]) {
        dragReceivers = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];

    NSMutableSet* newFigures = [NSMutableSet set];
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
//            [newFigures addObject:[faceOffsetTool addObject:face]];
//            [newFigures addObject:[faceRotationTool addObject:face]];
        }
    }
    
    [self notifyObservers:FiguresAdded infoObject:newFigures infoKey:FiguresKey];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    NSMutableSet* removedFigures = [NSMutableSet set];
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
//            [removedFigures addObject:[faceOffsetTool removeObject:face]];
//            [removedFigures addObject:[faceRotationTool removeObject:face]];
        }
    }
    
    [self notifyObservers:FiguresRemoved infoObject:removedFigures infoKey:FiguresKey];
}

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];
        Options* options = [windowController options];
        faceOffsetTool = [[FaceOffsetTool alloc] initWithOptions:options];
        faceRotationTool = [[FaceRotationTool alloc] initWithOptions:options];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        [selectionManager addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded];
        [selectionManager addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved];
    }
    
    return self;
}

- (void)addToolsHitByRay:(Ray3D *)theRay toList:(NSMutableArray *)toolList {
    if ([faceOffsetTool hitByRay:theRay])
        [toolList addObject:faceOffsetTool];
    if ([faceRotationTool hitByRay:theRay])
        [toolList addObject:faceRotationTool];
}

- (NSArray *)toolsHitByRay:(Ray3D *)theRay {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    [self addToolsHitByRay:theRay toList:result];
    return [result autorelease];
}

- (BOOL)startDrag:(Ray3D *)theRay {
    [self addToolsHitByRay:theRay toList:dragReceivers];
    if ([dragReceivers count] == 0)
        return NO;
    
    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager beginUndoGrouping];
    
    NSEnumerator* toolEn = [dragReceivers objectEnumerator];
    id <Tool> tool;
    while ((tool = [toolEn nextObject]))
        [tool startDrag:theRay];

    return YES;
}

- (void)drag:(Ray3D *)theRay {
    NSEnumerator* toolEn = [dragReceivers objectEnumerator];
    id <Tool> tool;
    while ((tool = [toolEn nextObject]))
        [tool drag:theRay];
}

- (void)endDrag:(Ray3D *)theRay {
    NSEnumerator* toolEn = [dragReceivers objectEnumerator];
    id <Tool> tool;
    while ((tool = [toolEn nextObject]))
        [tool endDrag:theRay];
    [dragReceivers removeAllObjects];

    NSUndoManager* undoManager = [[windowController document] undoManager];
    [undoManager setActionName:@"Change Texture Offset"];
    [undoManager endUndoGrouping];
}

- (BOOL)dragActive {
    return [dragReceivers count] > 0;
}

- (void)keyDown:(NSEvent *)theEvent {
    [faceOffsetTool keyDown:theEvent];
    [faceRotationTool keyDown:theEvent];
}

- (void)dealloc {
    [[windowController selectionManager] removeObserver:self];
    [dragReceivers release];
    [faceRotationTool release];
    [faceOffsetTool release];
    [windowController release];
    [super dealloc];
}

@end
