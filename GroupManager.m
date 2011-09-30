//
//  GroupManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GroupManager.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"

@interface GroupManager (private)

- (void)entitiesAdded:(NSNotification *)notification;
- (void)entitiesWereRemoved:(NSNotification *)notification;
- (void)brushesWillOrDidChange:(NSNotification *)notification;
- (void)documentCleared:(NSNotification *)notification;
- (void)documentLoaded:(NSNotification *)notification;

@end

@implementation GroupManager (private)

- (void)entitiesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    
    BOOL changed = NO;
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if ([GroupClassName isEqualToString:[entity classname]]) {
            [groups addObject:entity];
            if ([self isVisible:entity])
                visibleGroupCount++;
            changed |= YES;
        }
    }

    if (changed)
        [[NSNotificationCenter defaultCenter] postNotificationName:GroupsChanged object:self];
}

- (void)entitiesWereRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];

    BOOL changed = NO;
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if ([GroupClassName isEqualToString:[entity classname]]) {
            if ([self isVisible:entity])
                visibleGroupCount--;
            [groups removeObject:entity];
            changed |= YES;
        }
    }

    if (changed)
        [[NSNotificationCenter defaultCenter] postNotificationName:GroupsChanged object:self];
}

- (void)brushesWillOrDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> entity = [brush entity];
        if ([GroupClassName isEqualToString:[entity classname]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:GroupsChanged object:self];
            break;
        }
    }
}

- (void)documentCleared:(NSNotification *)notification {
    [groups removeAllObjects];
    visibleGroupCount = 0;
}

- (void)documentLoaded:(NSNotification *)notification {
    NSEnumerator* entityEn = [[map entities] objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if ([GroupClassName isEqualToString:[entity classname]]) {
            [groups addObject:entity];
            if ([self isVisible:entity])
                visibleGroupCount++;
        }
    }
}

@end

@implementation GroupManager

- (id)initWithMap:(MapDocument *)theMap {
    NSAssert(theMap != nil, @"map must not be nil");
    
    if ((self = [self init])) {
        map = theMap;
        groups = [[NSMutableArray alloc] init];
        visibleGroupCount = 0;

        NSNotificationCenter* center= [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entitiesAdded:) name:EntitiesAdded object:map];
        [center addObserver:self selector:@selector(entitiesWereRemoved:) name:EntitiesWereRemoved object:map];
        [center addObserver:self selector:@selector(brushesWillOrDidChange:) name:BrushesWillChange object:map];
        [center addObserver:self selector:@selector(brushesWillOrDidChange:) name:BrushesDidChange object:map];
        [center addObserver:self selector:@selector(documentCleared:) name:DocumentCleared object:map];
        [center addObserver:self selector:@selector(documentLoaded:) name:DocumentCleared object:map];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [groups release];
    [super dealloc];
}

- (NSArray *)groups {
    return groups;
}

- (void)setGroup:(id <Entity>)theGroup name:(NSString *)theName {
    NSAssert(theGroup != nil, @"group must not be nil");

    [map setEntity:theGroup propertyKey:GroupNameKey value:theName];
}

- (void)setGroup:(id <Entity>)theGroup visibility:(BOOL)theVisibility {
    NSAssert(theGroup != nil, @"group must not be nil");

    if (theVisibility == [self isVisible:theGroup])
        return;
    
    if (theVisibility) {
        [map setEntity:theGroup propertyKey:GroupVisibilityKey value:@"1"];
        visibleGroupCount++;
    } else {
        [map setEntity:theGroup propertyKey:GroupVisibilityKey value:@"0"];
        visibleGroupCount--;
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:GroupsChanged object:self];
}

- (BOOL)isVisible:(id <Entity>)theGroup {
    NSAssert(theGroup != nil, @"group must not be nil");
    NSString* value = [theGroup propertyForKey:GroupVisibilityKey];
    if (value == nil)
        return NO;
    
    return [value boolValue];
}

- (BOOL)allGroupsInvisible {
    return visibleGroupCount == 0;
}

@end
