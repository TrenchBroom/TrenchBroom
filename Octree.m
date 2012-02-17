/*
Copyright (C) 2010-2012 Kristian Duske

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

#import "Octree.h"
#import "OctreeNode.h"
#import "MapDocument.h"
#import "EntityDefinition.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@interface Octree (private)

- (void)entitiesAdded:(NSNotification *)notification;
- (void)entitiesWillBeRemoved:(NSNotification *)notification;
- (void)propertiesWillChange:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;
- (void)brushesAdded:(NSNotification *)notification;
- (void)brushesWillBeRemoved:(NSNotification *)notification;
- (void)brushesWillChange:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)documentLoaded:(NSNotification *)notification;
- (void)documentCleared:(NSNotification *)notification;

@end

@implementation Octree (private)

- (void)entitiesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    for (id <Entity> entity in entities)
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [root addObject:entity bounds:[entity bounds]];
}

- (void)entitiesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    for (id <Entity> entity in entities)
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            if (![root removeObject:entity bounds:[entity bounds]])
                [NSException raise:NSInvalidArgumentException format:@"Entity %@ was not removed from octree", entity];
}

- (void)propertiesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    for (id <Entity> entity in entities)
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [root removeObject:entity bounds:[entity bounds]];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    for (id <Entity> entity in entities)
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [root addObject:entity bounds:[entity bounds]];
}

- (void)brushesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    for (id <Brush> brush in brushes)
        [root addObject:brush bounds:[brush bounds]];
}

- (void)brushesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    for (id <Brush> brush in brushes)
        if (![root removeObject:brush bounds:[brush bounds]])
            [NSException raise:NSInvalidArgumentException format:@"Brush %@ was not removed from octree", brush];
}

- (void)brushesWillChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    for (id <Brush> brush in brushes)
        [root removeObject:brush bounds:[brush bounds]];
}

- (void)brushesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    for (id <Brush> brush in brushes)
        [root addObject:brush bounds:[brush bounds]];
}

- (void)documentLoaded:(NSNotification *)notification {
    MapDocument* map = [notification object];
    for (id <Entity> entity in [map entities]) {
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [root addObject:entity bounds:[entity bounds]];
        for (id <Brush> brush in [entity brushes])
            [root addObject:brush bounds:[brush bounds]];
    }
}

- (void)documentCleared:(NSNotification *)notification {
    [root release];
    
    MapDocument* map = [notification object];
    root = [[OctreeNode alloc] initWithMin:&[map worldBounds]->min max:&[map worldBounds]->max minSize:minSize];
}

@end

@implementation Octree

- (id)initWithMap:(MapDocument *)theMap minSize:(int)theMinSize {
    if ((self = [self init])) {
        minSize = theMinSize;
        root = [[OctreeNode alloc] initWithMin:&[theMap worldBounds]->min max:&[theMap worldBounds]->max minSize:minSize];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entitiesAdded:) name:EntitiesAdded object:theMap];
        [center addObserver:self selector:@selector(entitiesWillBeRemoved:) name:EntitiesWillBeRemoved object:theMap];
        [center addObserver:self selector:@selector(propertiesWillChange:) name:PropertiesWillChange object:theMap];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:theMap];
        [center addObserver:self selector:@selector(brushesAdded:) name:BrushesAdded object:theMap];
        [center addObserver:self selector:@selector(brushesWillBeRemoved:) name:BrushesWillBeRemoved object:theMap];
        [center addObserver:self selector:@selector(brushesWillChange:) name:BrushesWillChange object:theMap];
        [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:theMap];
        [center addObserver:self selector:@selector(documentLoaded:) name:DocumentLoaded object:theMap];
        [center addObserver:self selector:@selector(documentCleared:) name:DocumentCleared object:theMap];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [root release];
    [super dealloc];
}

- (NSArray *)pickObjectsWithRay:(const TRay *)ray {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    [root addObjectsForRay:ray to:result];
    return [result autorelease];
}

@end
