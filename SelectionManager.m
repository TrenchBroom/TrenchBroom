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
        faces = [[NSMutableArray alloc] init];
        partialBrushes = [[NSMutableArray alloc] init];
        brushes = [[NSMutableArray alloc] init];
        entities = [[NSMutableArray alloc] init];
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
            NSArray* removedEntities = [[NSArray alloc] initWithArray:entities];
            [self removeEntities:removedEntities record:record];
            [removedEntities release];
        }
        if ([brushes count] > 0) {
            NSArray* removedBrushes = [[NSArray alloc] initWithArray:brushes];
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
    
    NSArray* faceArray = [[NSArray alloc] initWithObjects:face, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:faceArray, SelectionFaces, nil];
    [faceArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addFaces:(NSArray *)theFaces record:(BOOL)record {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode != SM_FACES) {
        if ([entities count] > 0) {
            NSArray* removedEntities = [[NSArray alloc] initWithArray:entities];
            [self removeEntities:removedEntities record:record];
            [removedEntities release];
        }
        if ([brushes count] > 0) {
            NSArray* removedBrushes = [[NSArray alloc] initWithArray:brushes];
            [self removeBrushes:removedBrushes record:record];
            [removedBrushes release];
        }
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeFaces:theFaces record:record];
        [undoManager endUndoGrouping];
    }

    [faces addObjectsFromArray:theFaces];
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [partialBrushes addObject:[face brush]];
    mode = SM_FACES;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theFaces, SelectionFaces, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode == SM_FACES) {
        NSArray* removedFaces = [[NSArray alloc] initWithArray:faces];
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
    
    NSArray* brushArray = [[NSArray alloc] initWithObjects:brush, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:brushArray, SelectionBrushes, nil];
    [brushArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addBrushes:(NSArray *)theBrushes record:(BOOL)record {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode == SM_FACES) {
        NSArray* removedFaces = [[NSArray alloc] initWithArray:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    
    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeBrushes:theBrushes record:record];
        [undoManager endUndoGrouping];
    }
    
    [brushes addObjectsFromArray:theBrushes];
    if (mode == SM_ENTITIES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_BRUSHES;
    brushSelectionEntityValid = NO;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theBrushes, SelectionBrushes, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"entity must not be nil");
    
    if (record)
        [undoManager beginUndoGrouping];

    if (mode == SM_FACES) {
        NSArray* removedFaces = [[NSArray alloc] initWithArray:faces];
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
    
    NSArray* entityArray = [[NSArray alloc] initWithObjects:entity, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:entityArray, SelectionEntities, nil];
    [entityArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (void)addEntities:(NSArray *)theEntities record:(BOOL)record {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if (record)
        [undoManager beginUndoGrouping];
    
    if (mode == SM_FACES) {
        NSArray* removedFaces = [[NSArray alloc] initWithArray:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }

    if (record) {
        [[undoManager prepareWithInvocationTarget:self] removeEntities:theEntities record:record];
        [undoManager endUndoGrouping];
    }
    
    [entities addObjectsFromArray:theEntities];
    if (mode == SM_BRUSHES)
        mode = SM_BRUSHES_ENTITIES;
    else
        mode = SM_ENTITIES;
    brushSelectionEntityValid = NO;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theEntities, SelectionEntities, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionAdded object:self userInfo:userInfo];
    [userInfo release];
}

- (ESelectionMode)mode {
    return mode;
}

- (BOOL)isFaceSelected:(id <Face>)face {
    NSAssert(face != nil, @"face must not be nil");
    return [faces indexOfObjectIdenticalTo:face] != NSNotFound || [self isBrushSelected:[face brush]];
}

- (BOOL)isBrushSelected:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    return [brushes indexOfObjectIdenticalTo:brush] != NSNotFound || [self isEntitySelected:[brush entity]];
}

- (BOOL)isEntitySelected:(id <Entity>)entity {
    if (entity == nil)
        NSLog(@"asdf");
    NSAssert(entity != nil, @"entity must not be nil");
    return [entities indexOfObjectIdenticalTo:entity] != NSNotFound;
}

- (BOOL)isBrushPartiallySelected:(id <Brush>)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (mode != SM_FACES)
        return NO;
    
    return [partialBrushes indexOfObjectIdenticalTo:brush] != NSNotFound;
}

- (NSArray *)selectedEntities {
    return entities;
}

- (NSArray *)selectedBrushes {
    return brushes;
}

- (NSArray *)selectedFaces {
    return faces;
}

- (NSArray *)selectedBrushFaces {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    NSEnumerator* brushEn = [brushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [result addObjectsFromArray:[brush faces]];
    return [result autorelease];
}

- (NSArray *)partiallySelectedBrushes {
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
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addFace:face record:record];
    
    [faces removeObjectIdenticalTo:face];
    if ([faces count] == 0) {
        mode = SM_UNDEFINED;
        [partialBrushes removeAllObjects];
    } else {
        BOOL keepPartialBrush = NO;
        NSEnumerator* faceEn = [[[face brush] faces] objectEnumerator];
        id <Face> sibling;
        while ((sibling = [faceEn nextObject]) && !keepPartialBrush)
            keepPartialBrush = [faces indexOfObjectIdenticalTo:sibling] != NSNotFound;
        
        if (!keepPartialBrush)
            [partialBrushes removeObject:[face brush]];
    }
    
    NSArray* faceArray = [[NSArray alloc] initWithObjects:face, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:faceArray, SelectionFaces, nil];
    [faceArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeFaces:(NSArray *)theFaces record:(BOOL)record {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addFaces:theFaces record:record];

    NSEnumerator* faceEn = [theFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        [faces removeObjectIdenticalTo:face];
        
        BOOL keepPartialBrush = NO;
        NSEnumerator* faceEn = [[[face brush] faces] objectEnumerator];
        id <Face> sibling;
        while ((sibling = [faceEn nextObject]) && !keepPartialBrush)
            keepPartialBrush = [faces indexOfObjectIdenticalTo:sibling] != NSNotFound;
        
        if (!keepPartialBrush)
            [partialBrushes removeObject:[face brush]];
    }
    
    if ([faces count] == 0)
        mode = SM_UNDEFINED;
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theFaces, SelectionFaces, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeBrush:(id <Brush>)brush record:(BOOL)record {
    NSAssert(brush != nil, @"brush must not be nil");
    
    [brushes removeObjectIdenticalTo:brush];
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
    
    NSArray* brushArray = [[NSArray alloc] initWithObjects:brush, nil];
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:brushArray, SelectionBrushes, nil];
    [brushArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeBrushes:(NSArray *)theBrushes record:(BOOL)record {
    NSAssert(theBrushes != nil, @"brush set must not be nil");

    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [brushes removeObjectIdenticalTo:brush];
    
    if ([brushes count] == 0) {
        if ([entities count] == 0)
            mode = SM_UNDEFINED;
        else
            mode = SM_ENTITIES;
    }
    brushSelectionEntityValid = NO;
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addBrushes:theBrushes record:record];
    
    NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theBrushes, SelectionBrushes, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeEntity:(id <Entity>)entity record:(BOOL)record {
    NSAssert(entity != nil, @"entity must not be nil");
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addEntity:entity record:record];
    
    [entities removeObjectIdenticalTo:entity];
    
    if ([entities count] == 0) {
        if ([brushes count] > 0)
            mode = SM_BRUSHES;
        else
            mode = SM_UNDEFINED;
    }
    
    NSArray* entityArray = [[NSArray alloc] initWithObjects:entity, nil];
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] initWithObjectsAndKeys:entityArray, SelectionEntities, nil];
    [entityArray release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeEntities:(NSArray *)theEntities record:(BOOL)record {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if (record)
        [[undoManager prepareWithInvocationTarget:self] addEntities:theEntities record:record];

    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [entities removeObjectIdenticalTo:entity];
    
    if ([entities count] == 0) {
        if ([brushes count] > 0)
            mode = SM_BRUSHES;
        else
            mode = SM_UNDEFINED;
    }
    
    NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] initWithObjectsAndKeys:theEntities, SelectionEntities, nil];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:SelectionRemoved object:self userInfo:userInfo];
    [userInfo release];
}

- (void)removeAll:(BOOL)record {
    if ([faces count] > 0) {
        NSArray* removedFaces = [[NSArray alloc] initWithArray:faces];
        [self removeFaces:removedFaces record:record];
        [removedFaces release];
    }
    if ([brushes count] > 0) {
        NSArray* removedBrushes = [[NSArray alloc] initWithArray:brushes];
        [self removeBrushes:removedBrushes record:record];
        [removedBrushes release];
    }
    if ([entities count] > 0) {
        NSArray* removedEntities = [[NSArray alloc] initWithArray:entities];
        [self removeEntities:removedEntities record:record];
        [removedEntities release];
    }
}

@end
