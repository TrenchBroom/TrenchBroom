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
#import "Edge.h"
#import "Vector3f.h"

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

- (id)initWithUndoManager:(NSUndoManager *)theUndoManager {
    NSAssert(theUndoManager != nil, @"undo manager must not be nil");
    
    if (self = [self init]) {
        undoManager = [theUndoManager retain];
    }
    
    return self;
}

- (void)addFace:(id <Face>)face record:(BOOL)record {
    NSAssert(face != nil, @"face must not be nil");
    
    if ([faces containsObject:face])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_FACES && ([entities count] > 0 || [brushes count] > 0)) {
        if ([entities count] > 0) {
            NSSet* removedEntities = [[NSSet alloc] initWithSet:entities];
            [self removeEntities:removedEntities record:record];
            [removedEntities release];
        }
        if ([brushes count] > 0) {
            NSSet* removedBrushes = [[NSSet alloc] initWithSet:brushes];
            [self removeBrushes:removedBrushes record:record];
            [removedBrushes release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeFace:face record:record];
        [undoManager endUndoGrouping];
    }
    
    [faces addObject:face];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addFaces:(NSSet *)theFaces record:(BOOL)record {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    NSMutableSet* addedFaces = [[NSMutableSet alloc] initWithSet:theFaces];
    [addedFaces minusSet:faces];

    if ([addedFaces count] == 0) {
        [addedFaces release];
        return;
    }
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_FACES && ([entities count] > 0 || [brushes count] > 0)) {
        if ([entities count] > 0) {
            NSSet* removedEntities = [[NSSet alloc] initWithSet:entities];
            [self removeEntities:removedEntities record:record];
            [removedEntities release];
        }
        if ([brushes count] > 0) {
            NSSet* removedBrushes = [[NSSet alloc] initWithSet:brushes];
            [self removeBrushes:removedBrushes record:record];
            [removedBrushes release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeFaces:addedFaces record:record];
        [undoManager endUndoGrouping];
    }
    
    [faces unionSet:addedFaces];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:addedFaces forKey:SelectionFaces];
    [addedFaces release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if ([brushes containsObject:brush])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        if ([faces count] > 0) {
            NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
            [self removeFaces:removedFaces record:record];
            [removedFaces release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeBrush:brush record:record];
        [undoManager endUndoGrouping];
    }
    
    [brushes addObject:brush];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addBrushes:(NSSet *)theBrushes record:(BOOL)record {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    NSMutableSet* addedBrushes = [[NSMutableSet alloc] initWithSet:theBrushes];
    [addedBrushes minusSet:brushes];
    
    if ([addedBrushes count] == 0) {
        [addedBrushes release];
        return;
    }

    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        if ([faces count] > 0) {
            NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
            [self removeFaces:removedFaces record:record];
            [removedFaces release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeBrushes:theBrushes record:record];
        [undoManager endUndoGrouping];
    }
    
    [brushes unionSet:addedBrushes];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:addedBrushes forKey:SelectionBrushes];
    [addedBrushes release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"entity must not be nil");
 
    if ([entities containsObject:entity])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];

    if (mode != SM_GEOMETRY && [faces count] > 0) {
        if ([faces count] > 0) {
            NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
            [self removeFaces:removedFaces record:record];
            [removedFaces release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeEntity:entity record:record];
        [undoManager endUndoGrouping];
    }

    [entities addObject:entity];
    mode = SM_GEOMETRY;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
}

- (void)addEntities:(NSSet *)theEntities record:(BOOL)record {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    NSMutableSet* addedEntities = [[NSMutableSet alloc] initWithSet:theEntities];
    [addedEntities minusSet:entities];
    
    if ([addedEntities count] == 0) {
        [addedEntities release];
        return;
    }

    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_GEOMETRY && [faces count] > 0) {
        if ([faces count] > 0) {
            NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
            [self removeFaces:removedFaces record:record];
            [removedFaces release];
        }
    }

    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeEntities:theEntities record:record];
        [undoManager endUndoGrouping];
    }
    
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

- (BOOL)isVertexSelected:(Vertex *)vertex {
    NSAssert(vertex != nil, @"vertex must not be nil");
    NSEnumerator* edgeEn = [[vertex edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        if ([self isEdgeSelected:edge])
            return YES;
    
    return NO;
}

- (BOOL)isEdgeSelected:(Edge *)edge {
    NSAssert(edge != nil, @"edge must not be nil");
    return [self isFaceSelected:[edge leftFace]] || [self isFaceSelected:[edge rightFace]];
}

- (BOOL)isFaceSelected:(id <Face>)face {
    NSAssert(face != nil, @"face must not be nil");
    return [faces containsObject:face] || [self isBrushSelected:[face brush]];
}

- (BOOL)isBrushSelected:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    return [brushes containsObject:brush] || [self isEntitySelected:[brush entity]];
}

- (BOOL)isEntitySelected:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must not be nil");
    return [entities containsObject:entity];
}

- (BOOL)hasSelectedFaces:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
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

- (void)removeFace:(id <Face>)face record:(BOOL)record {
    NSAssert(face != nil, @"face must not be nil");
    
    if (![faces containsObject:face])
        return;
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addFace:face record:record];
    
    [faces removeObject:face];
    if ([faces count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:face] forKey:SelectionFaces];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeFaces:(NSSet *)theFaces record:(BOOL)record {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    NSMutableSet* removedFaces = [[NSMutableSet alloc] initWithSet:theFaces];
    [removedFaces intersectSet:faces];
    
    if ([removedFaces count] == 0) {
        [removedFaces release];
        return;
    }
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addFaces:removedFaces record:record];
    
    [faces minusSet:removedFaces];
    if ([faces count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:removedFaces forKey:SelectionFaces];
    [removedFaces release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (![brushes containsObject:brush])
        return;
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addBrush:brush record:record];
    
    [brushes removeObject:brush];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:brush] forKey:SelectionBrushes];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeBrushes:(NSSet *)theBrushes record:(BOOL)record {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    NSMutableSet* removedBrushes = [[NSMutableSet alloc] initWithSet:theBrushes];
    [removedBrushes intersectSet:brushes];
    
    if ([removedBrushes count] == 0) {
        [removedBrushes release];
        return;
    }
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addBrushes:removedBrushes record:record];
    
    [brushes minusSet:removedBrushes];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:removedBrushes forKey:SelectionBrushes];
    [removedBrushes release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"brush must not be nil");
    
    if (![entities containsObject:entity]);
        return;
        
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addEntity:entity record:record];
    
    [entities removeObject:entity];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:[NSSet setWithObject:entity] forKey:SelectionEntities];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeEntities:(NSSet *)theEntities record:(BOOL)record {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    NSMutableSet* removedEntities = [[NSMutableSet alloc] initWithSet:theEntities];
    [removedEntities intersectSet:entities];
    
    if ([removedEntities count] == 0) {
        [removedEntities release];
        return;
    }
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addEntities:removedEntities record:record];
    
    [entities minusSet:removedEntities];
    if ([brushes count] == 0 && [entities count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:removedEntities forKey:SelectionEntities];
    [removedEntities release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
}

- (void)removeAll:(BOOL)record {
    if ([faces count] > 0) {
        NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    if ([brushes count] > 0) {
        NSSet* removedBrushes = [[NSSet alloc] initWithSet:brushes];
        [self removeBrushes:removedBrushes record:record];
        [removedBrushes release];
    }
    if ([entities count] > 0) {
        NSSet* removedEntities = [[NSSet alloc] initWithSet:entities];
        [self removeEntities:removedEntities record:record];
        [removedEntities release];
    }
}

- (void)dealloc {
    [undoManager release];
    [entities release];
    [brushes release];
    [faces release];
    [super dealloc];
}

@end
