//
//  SelectionManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 20id <Brush>11 __MyCompanyName__. All rights reserved.
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

- (void)addFace:(id <Face>)face {
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

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }
    
    [faces addObject:face];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addFaces:(NSSet *)theFaces {
    if (theFaces == nil)
        [NSException raise:NSInvalidArgumentException format:@"face set must not be nil"];
    
    if (mode != SM_FACES && ([entities count] > 0 || [brushes count] > 0)) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        if ([entities count] > 0)
            [userInfo setObject:[NSSet setWithSet:entities] forKey:SelectionEntities];
        if ([brushes count] > 0)
            [userInfo setObject:[NSSet setWithSet:brushes] forKey:SelectionBrushes];
        
        [entities removeAllObjects];
        [brushes removeAllObjects];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }
    
    NSMutableSet* addedFaces = [[NSMutableSet alloc] initWithSet:theFaces];
    [addedFaces minusSet:faces];
    [faces unionSet:addedFaces];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:addedFaces forKey:SelectionFaces];
    [addedFaces release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addBrush:(id <Brush>)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }
    
    [brushes addObject:brush];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addBrushes:(NSSet *)theBrushes {
    if (theBrushes == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush set must not be nil"];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }
    
    NSMutableSet* addedBrushes = [[NSMutableSet alloc] initWithSet:theBrushes];
    [addedBrushes minusSet:brushes];
    [brushes unionSet:addedBrushes];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:addedBrushes forKey:SelectionBrushes];
    [addedBrushes release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addEntity:(id <Entity>)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
 
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }
    
    [entities addObject:entity];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addEntities:(NSSet *)theEntities {
    if (theEntities == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity set must not be nil"];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        NSDictionary* userInfo = [NSMutableDictionary dictionaryWithObject:[NSSet setWithSet:faces] forKey:SelectionFaces];
        [faces removeAllObjects];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    }

    NSMutableSet* addedEntities = [[NSMutableSet alloc] initWithSet:theEntities];
    [addedEntities minusSet:entities];
    [entities unionSet:addedEntities];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:addedEntities forKey:SelectionEntities];
    [addedEntities release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (ESelectionMode)mode {
    return mode;
}

- (BOOL)isFaceSelected:(id <Face>)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    return [faces containsObject:face];
}

- (BOOL)isBrushSelected:(id <Brush>)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];

    return [brushes containsObject:brush];
}

- (BOOL)isEntitySelected:(id <Entity>)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    return [entities containsObject:entity];
}

- (BOOL)hasSelectedFaces:(id <Brush>)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (mode != SM_FACES)
        return NO;
    
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    id <Face> face;
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
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [result addObjectsFromArray:[brush faces]];
    return [result autorelease];
}

- (Vector3f *)selectionCenter {
    switch ([self mode]) {
        case SM_FACES: {
            NSEnumerator* faceEn = [faces objectEnumerator];
            id <Face> face = [faceEn nextObject];
            Vector3f* center = [[Vector3f alloc] initWithFloatVector:[face center]];
            while ((face = [faceEn nextObject]))
                [center add:[face center]];
            
            [center scale:1.0f / [faces count]];
            return [center autorelease];
        }
        case SM_GEOMETRY: {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            Vector3f* center = [[Vector3f alloc] initWithFloatVector:[brush center]];
            while ((brush = [brushEn nextObject]))
                [center add:[brush center]];
            
            [center scale:1.0f / [brushes count]];
            return [center autorelease];
        }
    }
    return nil;
}

- (BOOL)hasSelection {
    return [self hasSelectedEntities] || [self hasSelectedBrushes] || [self hasSelectedFaces];
}

- (BOOL)hasSelectedEntities {
    return [entities count] > 0;
}

- (BOOL)hasSelectedBrushes {
    return [brushes count] > 0;
}

- (BOOL)hasSelectedFaces {
    return [faces count] > 0;
}

- (void)removeFace:(id <Face>)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    [faces removeObject:face];
    if ([faces count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeBrush:(id <Brush>)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brushes removeObject:brush];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeEntity:(id <Entity>)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    [entities removeObject:entity];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
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
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)dealloc {
    [entities release];
    [brushes release];
    [faces release];
    [super dealloc];
}

@end
