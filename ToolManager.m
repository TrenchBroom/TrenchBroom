//
//  ToolManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ToolManager.h"
#import "SelectionManager.h"
#import "FaceOffsetTool.h"
#import "Face.h"
#import "Tool.h"

NSString* const ToolsAdded = @"ToolsAdded";
NSString* const ToolsRemoved = @"ToolsRemoved";
NSString* const ToolsKey = @"Tools";

@implementation ToolManager

- (id)init {
    if (self = [super init]) {
        activeTools = [[NSMutableDictionary alloc] init];
        dragReceivers = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];

    NSMutableSet* newTools = [NSMutableSet set];
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
            NSMutableArray* toolsForFace = [[NSMutableArray alloc] init];
            [activeTools setObject:toolsForFace forKey:[face faceId]];
            [toolsForFace release];
            
            FaceOffsetTool* tool = [[FaceOffsetTool alloc] initWithFace:face selectionManager:selectionManager];
            [toolsForFace addObject:tool];
            [newTools addObject:tool];
            [tool release];
        }
    }
    
    [self notifyObservers:ToolsAdded infoObject:newTools infoKey:ToolsKey];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    NSMutableSet* removedTools = [NSMutableSet set];
    
    if (faces != nil) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
            [removedTools addObjectsFromArray:[activeTools objectForKey:[face faceId]]];
            [activeTools removeObjectForKey:[face faceId]];
        }
    }
    
    [self notifyObservers:ToolsRemoved infoObject:removedTools infoKey:ToolsKey];
}

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager {
    if (theSelectionManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"selection manager must not be nil"];
    
    if (self = [self init]) {
        selectionManager = [theSelectionManager retain];
        [selectionManager addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded];
        [selectionManager addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved];
    }
    
    return self;
}

- (void)addToolsHitByRay:(Ray3D *)theRay toList:(NSMutableArray *)toolList {
    NSEnumerator* toolsForFaceEn = [activeTools objectEnumerator];
    NSArray* toolsForFace;
    while ((toolsForFace = [toolsForFaceEn nextObject])) {
        NSEnumerator* toolEn = [toolsForFace objectEnumerator];
        id <Tool> tool;
        while ((tool = [toolEn nextObject]))
            if ([tool hitByRay:theRay])
                [toolList addObject:tool];
    }
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
}

- (BOOL)dragActive {
    return [dragReceivers count] > 0;
}

- (void)dealloc {
    [selectionManager removeObserver:self];
    [selectionManager release];
    [activeTools release];
    [dragReceivers release];
    [super dealloc];
}

@end
