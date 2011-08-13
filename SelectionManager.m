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
NSString* const SelectionVertices = @"SelectionVertices";

@implementation SelectionManager

- (id) init {
    if ((self = [super init])) {
        faces = [[NSMutableSet alloc] init];
        partialBrushes = [[NSMutableSet alloc] init];
        brushes = [[NSMutableSet alloc] init];
        entities = [[NSMutableSet alloc] init];
        mode = SM_UNDEFINED;
    }
    
    return self;
}

- (id)initWithUndoManager:(NSUndoManager *)theUndoManager {
    NSAssert(theUndoManager != nil, @"undo manager must not be nil");
    
    if ((self = [self init])) {
        undoManager = [theUndoManager retain];
    }
    
    return self;
}

- (void)dealloc {
    [undoManager release];
    [entities release];
    [brushes release];
    [faces release];
    [partialBrushes release];
    [super dealloc];
}

- (void)addFace:(id <Face>)face record:(BOOL)record {
    NSAssert(face != nil, @"face must not be nil");
    
    if ([faces containsObject:face])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_FACES) {
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
    [partialBrushes addObject:[face brush]];
    mode = SM_FACES;
    
    NSSet* faceSet = [[NSSet alloc] initWithObjects:face, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:faceSet, SelectionFaces, nil];
    [faceSet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
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
    
    if (mode != SM_FACES) {
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
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [partialBrushes addObject:[face brush]];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:addedFaces, SelectionFaces, nil];
    [addedFaces release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if ([brushes containsObject:brush])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode == SM_FACES) {
        NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeBrush:brush record:record];
        [undoManager endUndoGrouping];
    }
    
    [brushes addObject:brush];
    if (mode == SM_ENTITIES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_BRUSHES;
    brushSelectionEntityValid = NO;
    
    NSSet* brushSet = [[NSSet alloc] initWithObjects:brush, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:brushSet, SelectionBrushes, nil];
    [brushSet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
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
    
    if (mode == SM_FACES) {
        NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeBrushes:addedBrushes record:record];
        [undoManager endUndoGrouping];
    }
    
    [brushes unionSet:addedBrushes];
    if (mode == SM_ENTITIES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_BRUSHES;
    brushSelectionEntityValid = NO;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:addedBrushes, SelectionBrushes, nil];
    [addedBrushes release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"entity must not be nil");
 
    if ([entities containsObject:entity])
        return;
    
    if (record)
        [undoManager beginUndoGrouping];

    if (mode == SM_FACES) {
        NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeEntity:entity record:record];
        [undoManager endUndoGrouping];
    }

    [entities addObject:entity];
    if (mode == SM_BRUSHES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_ENTITIES;
    brushSelectionEntityValid = NO;
    
    NSSet* entitySet = [[NSSet alloc] initWithObjects:entity, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:entitySet, SelectionEntities, nil];
    [entitySet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
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
    
    if (mode == SM_FACES) {
        NSSet* removedFaces = [[NSSet alloc] initWithSet:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }

    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeEntities:addedEntities record:record];
        [undoManager endUndoGrouping];
    }
    
    [entities unionSet:addedEntities];
    if (mode == SM_BRUSHES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_ENTITIES;
    brushSelectionEntityValid = NO;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:addedEntities, SelectionEntities, nil];
    [addedEntities release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (ESelectionMode)mode {
    return mode;
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

- (BOOL)isBrushPartiallySelected:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (mode != SM_FACES)
        return NO;
    
    return [partialBrushes containsObject:brush];
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

- (NSSet *)partiallySelectedBrushes {
    return partialBrushes;
}

- (id <Entity>)brushSelectionEntity {
    if (mode != SM_BRUSHES)
        return nil;

    if (!brushSelectionEntityValid) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush = [brushEn nextObject];
        brushSelectionEntity = [brush entity];
        while ((brush = [brushEn nextObject]) && brushSelectionEntity != nil) {
            if ([brush entity] != brushSelectionEntity)
                brushSelectionEntity = nil;
            
        }
        brushSelectionEntityValid = YES;
    }
    
    return brushSelectionEntity;
}

- (BOOL)selectionCenter:(TVector3f *)result {
    switch ([self mode]) {
        case SM_FACES: {
            NSEnumerator* faceEn = [faces objectEnumerator];
            id <Face> face = [faceEn nextObject];
            *result = *[face center];
            while ((face = [faceEn nextObject]))
                addV3f(result, [face center], result);

            scaleV3f(result, 1.0f / [faces count], result);
            return YES;
        }
        case SM_BRUSHES: {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            *result = *[brush center];
            while ((brush = [brushEn nextObject]))
                addV3f(result, [brush center], result);

            scaleV3f(result, 1.0f / [brushes count], result);
            return YES;
        }
        case SM_ENTITIES: {
            NSEnumerator* entityEn = [entities objectEnumerator];
            id <Entity> entity = [entityEn nextObject];
            *result = *[entity center];
            while ((entity = [entityEn nextObject]))
                addV3f(result, [entity center], result);
            
            scaleV3f(result, 1.0f / [entities count], result);
            return YES;
        }
        case SM_BRUSHES_ENTITIES: {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            *result = *[brush center];
            while ((brush = [brushEn nextObject]))
                addV3f(result, [brush center], result);

            NSEnumerator* entityEn = [entities objectEnumerator];
            id <Entity> entity;
            while ((entity = [entityEn nextObject]))
                addV3f(result, [entity center], result);
            
            scaleV3f(result, 1.0f / ([brushes count] + [entities count]), result);
            return YES;
        }
        default:
            return NO;
    }
}

- (BOOL)selectionBounds:(TBoundingBox *)result {
    switch ([self mode]) {
        case SM_BRUSHES: {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            *result = *[brush bounds];
            while ((brush = [brushEn nextObject]))
                mergeBoundsWithBounds(result, [brush bounds], result);
            return YES;
        }
        case SM_ENTITIES: {
            NSEnumerator* entityEn = [entities objectEnumerator];
            id <Entity> entity = [entityEn nextObject];
            *result = *[entity bounds];
            while ((entity = [entityEn nextObject]))
                mergeBoundsWithBounds(result, [entity bounds], result);
            return YES;
        }
        case SM_BRUSHES_ENTITIES: {
            NSEnumerator* brushEn = [brushes objectEnumerator];
            id <Brush> brush = [brushEn nextObject];
            *result = *[brush bounds];
            while ((brush = [brushEn nextObject]))
                mergeBoundsWithBounds(result, [brush bounds], result);
            
            NSEnumerator* entityEn = [entities objectEnumerator];
            id <Entity> entity;
            while ((entity = [entityEn nextObject]))
                mergeBoundsWithBounds(result, [entity bounds], result);
            
            return YES;
        }
        case SM_FACES: {
            NSEnumerator* faceEn = [faces objectEnumerator];
            id <Face> face = [faceEn nextObject];
            *result = *[[face brush] bounds];
            while ((face = [faceEn nextObject]))
                mergeBoundsWithBounds(result, [[face brush] bounds], result);
            
            return YES;
        }
        default:
            result->min = NullVector;
            result->max = NullVector;
            return NO;
    }
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
    if ([faces count] == 0) {
        mode = SM_UNDEFINED;
        [partialBrushes removeAllObjects];
    } else {
        NSMutableSet* remainingFaces = [[NSMutableSet alloc] initWithArray:[[face brush] faces]];
        [remainingFaces intersectSet:faces];
        if ([remainingFaces count] == 0)
            [partialBrushes removeObject:[face brush]];
        [remainingFaces release];
    }
    
    NSSet* faceSet = [[NSSet alloc] initWithObjects:face, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:faceSet, SelectionFaces, nil];
    [faceSet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
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
    if ([faces count] == 0) {
        mode = SM_UNDEFINED;
        [partialBrushes removeAllObjects];
    } else {
        NSMutableSet* remainingFaces = [[NSMutableSet alloc] init];
        NSEnumerator* faceEn = [theFaces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            [remainingFaces addObjectsFromArray:[[face brush] faces]];
            [remainingFaces intersectSet:faces];
            if ([remainingFaces count] == 0)
                [partialBrushes removeObject:[face brush]];
            [remainingFaces removeAllObjects];
        }

        [remainingFaces release];
    }
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:removedFaces, SelectionFaces, nil];
    [removedFaces release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (![brushes containsObject:brush])
        return;
    
    [brushes removeObject:brush];
    if ([brushes count] == 0) {
        if ([entities count] == 0) {
            mode = SM_UNDEFINED;
        } else {
            mode = SM_ENTITIES;
        }
    }
    brushSelectionEntityValid = NO;
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addBrush:brush record:record];
    
    NSSet* brushSet = [[NSSet alloc] initWithObjects:brush, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:brushSet, SelectionBrushes, nil];
    [brushSet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeBrushes:(NSSet *)theBrushes record:(BOOL)record {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    NSMutableSet* removedBrushes = [[NSMutableSet alloc] initWithSet:theBrushes];
    [removedBrushes intersectSet:brushes];
    
    if ([removedBrushes count] == 0) {
        [removedBrushes release];
        return;
    }
    
    [brushes minusSet:removedBrushes];
    if ([brushes count] == 0) {
        if ([entities count] == 0)
            mode = SM_UNDEFINED;
        else
            mode = SM_ENTITIES;
    }
    brushSelectionEntityValid = NO;
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addBrushes:removedBrushes record:record];
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:removedBrushes, SelectionBrushes, nil];
    [removedBrushes release];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"entity must not be nil");
    
    if (![entities containsObject:entity])
        return;
        
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addEntity:entity record:record];
    
    [entities removeObject:entity];
    
    if ([entities count] == 0) {
        if ([brushes count] > 0)
            mode = SM_BRUSHES;
        else
            mode = SM_UNDEFINED;
    }
    
    NSSet* entitySet = [[NSSet alloc] initWithObjects:entity, nil];
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] initWithObjectsAndKeys:entitySet, SelectionEntities, nil];
    [entitySet release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
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
    if ([entities count] == 0) {
        if ([brushes count] > 0)
            mode = SM_BRUSHES;
        else
            mode = SM_UNDEFINED;
    }
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] initWithObjectsAndKeys:removedEntities, SelectionEntities, nil];
    [removedEntities release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
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

@end
