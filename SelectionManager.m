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

NSString* const SelectionEntities = @"SelectionEntities";
NSString* const SelectionBrushes = @"SelectionBrushes";
NSString* const SelectionFaces = @"SelectionFaces";

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
    
    if (mode != SM_FACES && ([entities count] > 0 || [brushes count] > 0)) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        if ([entities count] > 0)
            [userInfo setObject:[NSSet setWithSet:entities] forKey:SelectionEntities];
        if ([brushes count] > 0)
            [userInfo setObject:[NSSet setWithSet:brushes] forKey:SelectionBrushes];
        
        [entities removeAllObjects];
        [brushes removeAllObjects];

        [self notifyObservers:SelectionRemoved userInfo:userInfo];
    }
    
    [faces addObject:face];
    mode = SM_FACES;
    
    [self notifyObservers:SelectionAdded infoObject:[NSSet setWithObject:face] infoKey:SelectionFaces];
}

- (void)addBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];
        [self notifyObservers:SelectionRemoved userInfo:userInfo];
    }
    
    [brushes addObject:brush];
    mode = SM_GEOMETRY;
    
    [self notifyObservers:SelectionAdded infoObject:[NSSet setWithObject:brush] infoKey:SelectionBrushes];
}

- (void)addEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
 
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];
        [self notifyObservers:SelectionRemoved userInfo:userInfo];
    }
    
    [entities addObject:entity];
    mode = SM_GEOMETRY;
    
    [self notifyObservers:SelectionAdded infoObject:[NSSet setWithObject:entity] infoKey:SelectionEntities];
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

- (NSSet *)selectedBrushFaces {
    NSMutableSet* result = [[NSMutableSet alloc] init];
    NSEnumerator* brushEn = [brushes objectEnumerator];
    Brush* brush;
    while ((brush = [brushEn nextObject]))
        [result addObjectsFromArray:[brush faces]];
    return [result autorelease];
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
    
    [self notifyObservers:SelectionRemoved infoObject:[NSSet setWithObject:face] infoKey:SelectionFaces];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brushes removeObject:brush];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    [self notifyObservers:SelectionRemoved infoObject:[NSSet setWithObject:brush] infoKey:SelectionBrushes];
}

- (void)removeEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    [entities removeObject:entity];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    [self notifyObservers:SelectionRemoved infoObject:[NSSet setWithObject:entity] infoKey:SelectionEntities];
}

- (void)removeAll {
    if (![self hasSelection])
        return;
    
    NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
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
    
    [self notifyObservers:SelectionRemoved userInfo:userInfo];
}

- (void)dealloc {
    [entities release];
    [brushes release];
    [faces release];
    [super dealloc];
}

@end
