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

NSString* const SelectionChanged = @"SelectionChanged";

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
    
    if (mode != SM_FACES) {
        [entities removeAllObjects];
        [brushes removeAllObjects];
    }
    [faces addObject:face];
    mode = SM_FACES;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)addBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (mode != SM_GEOMETRY)
        [faces removeAllObjects];
    [brushes addObject:brush];
    mode = SM_GEOMETRY;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)addEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
 
    if (mode != SM_GEOMETRY)
        [faces removeAllObjects];
    [entities addObject:entity];
    mode = SM_GEOMETRY;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
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
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brushes removeObject:brush];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)removeEntity:(Entity *)entity {
    if (entity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    [entities removeObject:entity];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)removeAll {
    [faces removeAllObjects];
    [brushes removeAllObjects];
    [entities removeAllObjects];
    mode = SM_UNDEFINED;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotification:[NSNotification notificationWithName:SelectionChanged object:self]];
}

- (void)dealloc {
    [entities release];
    [brushes release];
    [faces release];
    [super dealloc];
}

@end
