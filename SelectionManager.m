//
//  SelectionManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectionManager.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

NSString* const SelectionAdded = @"SelectionAdded";
NSString* const SelectionRemoved = @"SelectionRemoved";

@implementation SelectionManager

- (id) init {
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
        brushes = [[NSMutableSet alloc] init];
        entities = [[NSMutableSet alloc] init];
        mode = SM_UNDEFINED;
    }
    
    return self;
}

- (void)addFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSMutableDictionary* userInfo = nil;
    
    if (mode != SM_FACES) {
        if ([entities count] > 0 || [brushes count] > 0) {
            userInfo = [[NSMutableDictionary alloc] init];
            if ([entities count] > 0)
                [userInfo setObject:[NSSet setWithSet:entities] forKey:SelectionEntities];
            if ([brushes count] > 0)
                [userInfo setObject:[NSSet setWithSet:brushes] forKey:SelectionBrushes];

            [entities removeAllObjects];
            [brushes removeAllObjects];
            
            [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
            [userInfo release];
        } else {
            [entities removeAllObjects];
            [brushes removeAllObjects];
        }
    }
    
    
    [faces addObject:face];
    mode = SM_FACES;
    
    userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    [center postNotification:[NSNotification notificationWithName:SelectionAdded object:self userInfo:userInfo]];
    [userInfo release];
}

- (void)addBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSMutableDictionary* userInfo = nil;
    
    if (mode != SM_GEOMETRY) {
        userInfo= [[NSMutableDictionary alloc] init];
        [userInfo setObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        
        [faces removeAllObjects];

        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
    [brushes addObject:brush];
    mode = SM_GEOMETRY;
    
    userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    [center postNotification:[NSNotification notificationWithName:SelectionAdded object:self userInfo:userInfo]];
    [userInfo release];
}

- (void)addEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
 
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSMutableDictionary* userInfo = nil;

    if (mode != SM_GEOMETRY) {
        userInfo= [[NSMutableDictionary alloc] init];
        [userInfo setObject:[NSSet setWithSet:faces] forKey:SelectionFaces];

        [faces removeAllObjects];
        
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
    [entities addObject:entity];
    mode = SM_GEOMETRY;
    
    userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    [center postNotification:[NSNotification notificationWithName:SelectionAdded object:self userInfo:userInfo]];
    [userInfo release];
}

- (ESelectionMode)mode {
    return mode;
}

- (BOOL)isFaceSelected:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    return [faces containsObject:face];
}

- (BOOL)isBrushSelected:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];

    return [brushes containsObject:brush];
}

- (BOOL)isEntitySelected:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    return [entities containsObject:entity];
}

- (BOOL)hasSelectedFaces:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (mode != SM_FACES)
        return NO;
    
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        if ([self isFaceSelected:face])
            return YES;
    
    return NO;
}

- (NSSet *)selectedEntities {
    return entities;
}

- (NSSet *)selectedBrushes {
    return brushes;
}

- (NSSet *)selectedFaces {
    return faces;
}

- (BOOL)hasSelection {
    return [entities count] > 0 || [brushes count] > 0 || [faces count] > 0;
}

- (void)removeFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    [faces removeObject:face];
    if ([faces count] == 0)
        mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    [center postNotification:[NSNotification notificationWithName:SelectionRemoved object:self userInfo:userInfo]];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brushes removeObject:brush];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    [center postNotification:[NSNotification notificationWithName:SelectionRemoved object:self userInfo:userInfo]];
}

- (void)removeEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    [entities removeObject:entity];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    [center postNotification:[NSNotification notificationWithName:SelectionRemoved object:self userInfo:userInfo]];
}

- (void)removeAll {
    if (![self hasSelection])
        return;
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
    if ([faces count] > 0)
        [userInfo setObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
    if ([brushes count] > 0)
        [userInfo setObject:[NSSet setWithSet:brushes] forKey:SelectionBrushes];
    if ([entities count] > 0)
        [userInfo setObject:[NSSet setWithSet:entities] forKey:SelectionEntities];
    
    [faces removeAllObjects];
    [brushes removeAllObjects];
    [entities removeAllObjects];
    mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionRemoved object:self userInfo:userInfo]];
    [userInfo release];
}

- (void)dealloc {
    [entities release];
    [brushes release];
    [faces release];
    [super dealloc];
}

@end
