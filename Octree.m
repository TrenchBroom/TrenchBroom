//
//  Octree.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Octree.h"
#import "OctreeNode.h"
#import "MapDocument.h"
#import "EntityDefinition.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@implementation Octree

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
        [root addObject:entity bounds:[entity bounds]];
}

- (void)entityWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
        if (![root removeObject:entity bounds:[entity bounds]])
            [NSException raise:NSInvalidArgumentException format:@"Entity %@ was not removed from octree", entity];
}

- (void)propertiesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    
    if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
        [root removeObject:entity bounds:[entity bounds]];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Entity> entity = [userInfo objectForKey:EntityKey];
    
    if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
        [root addObject:entity bounds:[entity bounds]];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    [root addObject:brush bounds:[brush bounds]];
}

- (void)brushWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    if (![root removeObject:brush bounds:[brush bounds]])
        [NSException raise:NSInvalidArgumentException format:@"Brush %@ was not removed from octree", brush];
}

- (void)brushWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    [root removeObject:brush bounds:[brush bounds]];
}

- (void)brushDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    id <Brush> brush = [userInfo objectForKey:BrushKey];
    [root addObject:brush bounds:[brush bounds]];
}

- (id)initWithDocument:(MapDocument *)theDocument minSize:(int)theMinSize {
    if ((self = [self init])) {
        map = theDocument;
        
        int w = [map worldSize] / 2;
        TVector3i min = {-w, -w, -w};
        TVector3i max = {+w, +w, +w};

        root = [[OctreeNode alloc] initWithMin:&min max:&max minSize:theMinSize];
        
        NSEnumerator* entityEn = [[map entities] objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
                [root addObject:entity bounds:[entity bounds]];
            
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject]))
                [root addObject:brush bounds:[brush bounds]];
        }
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entityAdded:) name:EntityAdded object:map];
        [center addObserver:self selector:@selector(entityWillBeRemoved:) name:EntityWillBeRemoved object:map];
        [center addObserver:self selector:@selector(propertiesWillChange:) name:PropertiesWillChange object:map];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesWillChange object:map];
        [center addObserver:self selector:@selector(brushAdded:) name:BrushAdded object:map];
        [center addObserver:self selector:@selector(brushWillBeRemoved:) name:BrushWillBeRemoved object:map];
        [center addObserver:self selector:@selector(brushWillChange:) name:BrushWillChange object:map];
        [center addObserver:self selector:@selector(brushDidChange:) name:BrushDidChange object:map];
    }
    
    return self;
}

- (NSArray *)pickObjectsWithRay:(TRay *)ray {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    [root addObjectsForRay:ray to:result];
    return [result autorelease];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [root release];
    [super dealloc];
}
@end
