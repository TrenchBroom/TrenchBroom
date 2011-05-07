//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapDocument.h"
#import "EntityDefinitionManager.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "MutableFace.h"
#import "TextureManager.h"
#import "TextureCollection.h"
#import "Texture.h"
#import "Picker.h"
#import "GLResources.h"
#import "WadLoader.h"
#import "MapWindowController.h"
#import "ProgressWindowController.h"
#import "MapParser.h"

NSString* const FaceWillChange      = @"FaceWillChange";
NSString* const FaceDidChange       = @"FaceDidChange";
NSString* const FaceKey             = @"Face";

NSString* const BrushAdded          = @"BrushAdded";
NSString* const BrushWillBeRemoved  = @"BrushWillBeRemoved";
NSString* const BrushWillChange     = @"BrushWillChange";
NSString* const BrushDidChange      = @"BrushDidChange";
NSString* const BrushKey            = @"Brush";

NSString* const EntityAdded         = @"EntityAdded";
NSString* const EntityWillBeRemoved = @"EntityWillBeRemoved";
NSString* const EntityKey           = @"Entity";

NSString* const PropertyAdded       = @"PropertyAdded";
NSString* const PropertyRemoved     = @"PropertyRemoved";
NSString* const PropertyWillChange  = @"PropertyWillChange";
NSString* const PropertyDidChange   = @"PropertyDidChange";
NSString* const PropertyKeyKey      = @"PropertyKey";
NSString* const PropertyOldValueKey = @"PropertyOldValue";
NSString* const PropertyNewValueKey = @"PropertyNewValue";

@implementation MapDocument

- (id)init {
    if (self = [super init]) {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* definitionPath = [mainBundle pathForResource:@"quake" ofType:@"def"];
        entityDefinitionManager = [[EntityDefinitionManager alloc] initWithDefinitionFile:definitionPath];

        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
        worldSize = 8192;
        postNotifications = YES;

        picker = [[Picker alloc] initWithDocument:self];
        glResources = [[GLResources alloc] init];
    }
    
    return self;
}

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapDocument"];
	[self addWindowController:controller];
    [controller release];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (void)refreshWadFiles {
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
    NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];

    TextureManager* textureManager = [glResources textureManager];
    [textureManager removeAllTextureCollections];
    
    NSString* wads = [[self worldspawn:NO] propertyForKey:@"wad"];
    if (wads != nil) {
        NSArray* wadPaths = [wads componentsSeparatedByString:@";"];
        for (int i = 0; i < [wadPaths count]; i++) {
            NSString* wadPath = [[wadPaths objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSFileManager* fileManager = [NSFileManager defaultManager];
            if ([fileManager fileExistsAtPath:wadPath]) {
                int slashIndex = [wadPath rangeOfString:@"/" options:NSBackwardsSearch].location;
                NSString* wadName = [wadPath substringFromIndex:slashIndex + 1];
                
                WadLoader* wadLoader = [[WadLoader alloc] init];
                Wad* wad = [wadLoader loadFromData:[NSData dataWithContentsOfMappedFile:wadPath] wadName:wadName];
                [wadLoader release];
                
                TextureCollection* collection = [[TextureCollection alloc] initName:wadPath palette:palette wad:wad];
                [textureManager addTextureCollection:collection];
                [collection release];
            }
        }
    }
    [palette release];
    
    [self updateTextureUsageCounts];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    ProgressWindowController* pwc = [[ProgressWindowController alloc] initWithWindowNibName:@"ProgressWindow"];
    [[pwc window] makeKeyAndOrderFront:self];
    [[pwc label] setStringValue:@"Loading map file..."];
    
    NSProgressIndicator* indicator = [pwc progressIndicator];
    [indicator setIndeterminate:NO];
    [indicator setUsesThreadedAnimation:YES];
    
    [[self undoManager] disableUndoRegistration];
    [self setPostNotifications:NO];
    
    MapParser* parser = [[MapParser alloc] initWithData:data];
    [parser parseMap:self withProgressIndicator:indicator];
    [parser release];

    // set the entity definitions
    NSEnumerator* entityEn = [entities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject]))
        [self setEntityDefinition:entity];

    [self setPostNotifications:YES];
    [[self undoManager] enableUndoRegistration];
    
    [pwc close];
    [pwc release];
    
    [picker release];
    picker = [[Picker alloc] initWithDocument:self];
    [self refreshWadFiles];
    
    return YES;
}

- (id <Entity>)worldspawn:(BOOL)create {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        NSEnumerator* en = [entities objectEnumerator];
        while ((worldspawn = [en nextObject]))
            if ([worldspawn isWorldspawn])
                break;
    }
    
    if (worldspawn == nil && create)
        worldspawn = [self createEntityWithClassname:WorldspawnClassname];
    
    return worldspawn;
}

- (id <Entity>)createEntityWithClassname:(NSString *)classname {
    NSAssert(classname != nil, @"class name must not be nil");
    
    EntityDefinition* entityDefinition = [entityDefinitionManager definitionForName:classname];
    if (entityDefinition == nil) {
        NSLog(@"No entity definition for class name '&@'", classname);
        return nil;
    }
    
    MutableEntity* entity = [[MutableEntity alloc] init];
    [entity setProperty:ClassnameKey value:classname];
    [entity setEntityDefinition:entityDefinition];

    [self addEntity:entity];
    return [entity autorelease];
}

- (id <Entity>)createEntityWithProperties:(NSDictionary *)properties {
    NSAssert(properties != nil, @"property dictionary must not be nil");
    
    NSString* classname = [properties objectForKey:ClassnameKey];
    NSAssert(classname != nil, @"property dictionary must contain classname property");
    
    EntityDefinition* entityDefinition = [entityDefinitionManager definitionForName:classname];
    if (entityDefinition == nil) {
        NSLog(@"No entity definition for class name '&@'", classname);
        return nil;
    }
    
    MutableEntity* entity = [[MutableEntity alloc] initWithProperties:properties];
    [entity setEntityDefinition:entityDefinition];
    
    [self addEntity:entity];
    return [entity autorelease];
}

- (void)addEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(worldspawn == nil || ![theEntity isWorldspawn], @"cannot overwrite worldspawn entity");
    
    [[[self undoManager] prepareWithInvocationTarget:self] removeEntity:theEntity];
    
    [entities addObject:theEntity];
    [theEntity setMap:self];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntity forKey:EntityKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)removeEntity:(MutableEntity *)theEntity {
    [[[self undoManager] prepareWithInvocationTarget:self] addEntity:theEntity];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntity forKey:EntityKey];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityWillBeRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [theEntity setMap:nil];
    [entities removeObject:theEntity];
    if (worldspawn == theEntity)
        worldspawn = nil;
}

- (void)setEntity:(id <Entity>)entity propertyKey:(NSString *)key value:(NSString *)value {
    NSString* oldValue = [entity propertyForKey:key];
    
    if (oldValue == nil) {
        if (value == nil)
            return;
    } else if ([oldValue isEqualToString:value])
        return;
    
    [[[self undoManager] prepareWithInvocationTarget:self] setEntity:entity propertyKey:key value:oldValue];
    
    NSMutableDictionary* userInfo = nil;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:entity forKey:EntityKey];
        [userInfo setObject:key forKey:PropertyKeyKey];
        if (oldValue != nil)
            [userInfo setObject:oldValue forKey:PropertyOldValueKey];
        if (value != nil)
            [userInfo setObject:value forKey:PropertyNewValueKey];

        if (oldValue != nil && value != nil) {
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:PropertyWillChange object:self userInfo:userInfo];
        }
    }
    
    MutableEntity* mutableEntity = (MutableEntity *)entity;
    if (value == nil)
        [mutableEntity removeProperty:key];
    else
        [mutableEntity setProperty:key value:value];

    if ([key isEqualToString:@"wad"])
        [self refreshWadFiles];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        if (oldValue == nil && value != nil)
            [center postNotificationName:PropertyAdded object:self userInfo:userInfo];
        else if (oldValue != nil && value == nil)
            [center postNotificationName:PropertyRemoved object:self userInfo:userInfo];
        else
            [center postNotificationName:PropertyDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setEntityDefinition:(id <Entity>)entity {
    MutableEntity* mutableEntity = (MutableEntity *)entity;

    NSString* classname = [mutableEntity classname];
    if (classname != nil) {
        EntityDefinition* entityDefinition = [entityDefinitionManager definitionForName:classname];
        if (entityDefinition != nil) {
            [mutableEntity setEntityDefinition:entityDefinition];
        } else {
            NSLog(@"Warning: no entity definition found for entity with id %@ and classname '%@' (line %i)", [entity entityId], classname, [mutableEntity filePosition]);
        }
    } else {
        NSLog(@"Warning: entity with id %@ is missing classname property (line %i)", [entity entityId], [mutableEntity filePosition]);
    }
}

- (void)translateEntity:(id <Entity>)entity xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    NSAssert(entity != nil, @"entity must not be nil");
    
    EntityDefinition* entityDefinition = [entity entityDefinition];
    if (entityDefinition == nil)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateEntity:entity xDelta:-xDelta yDelta:-yDelta zDelta:-zDelta];

    if ([entityDefinition type] == EDT_POINT) {
        TVector3i* origin = [entity origin];
        NSString* newOrigin = [NSString stringWithFormat:@"%i %i %i", origin->x + xDelta, origin->y + yDelta, origin->z + zDelta];
        [self setEntity:entity propertyKey:OriginKey value:newOrigin];
    } else if ([entityDefinition type] == EDT_BRUSH) {
        [undoManager beginUndoGrouping];
        
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject]))
            [self translateBrush:brush xDelta:xDelta yDelta:yDelta zDelta:zDelta];
        
        [undoManager endUndoGrouping];
    }
}

- (void)deleteEntity:(id <Entity>)entity {
    [self removeEntity:entity];
}

- (void)addTextureWad:(NSString *)wadPath {
    NSAssert(wadPath != nil, @"wad path must not be nil");
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:wadPath]) {
        int slashIndex = [wadPath rangeOfString:@"/" options:NSBackwardsSearch].location;
        NSString* wadName = [wadPath substringFromIndex:slashIndex + 1];
        
        WadLoader* wadLoader = [[WadLoader alloc] init];
        Wad* wad = [wadLoader loadFromData:[NSData dataWithContentsOfMappedFile:wadPath] wadName:wadName];
        [wadLoader release];
        
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];
        
        TextureCollection* collection = [[TextureCollection alloc] initName:wadPath palette:palette wad:wad];
        [palette release];

        TextureManager* textureManager = [glResources textureManager];
        [textureManager addTextureCollection:collection];
        [collection release];
        
        MutableEntity* wc = [self worldspawn:YES];
        [wc setProperty:@"wad" value:[textureManager wadProperty]];
        
        [self updateTextureUsageCounts];
    }
}

- (void)removeTextureWad:(NSString *)wadPath {
    NSAssert(wadPath != nil, @"wad path must not be nil");

    TextureManager* textureManager = [glResources textureManager];
    [textureManager removeTextureCollection:wadPath];
    
    MutableEntity* wc = [self worldspawn:YES];
    [wc setProperty:@"wad" value:[textureManager wadProperty]];

    [self updateTextureUsageCounts];
}

- (void)updateTextureUsageCounts {
    TextureManager* textureManager = [glResources textureManager];
    [textureManager resetUsageCounts];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                Texture* texture = [textureManager textureForName:[face texture]];
                if (texture != nil)
                    [texture incUsageCount];
            }
        }
    }
}

- (NSArray *)entities {
    return entities;
}

- (void)setFace:(id <Face>)face xOffset:(int)xOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xOffset:[face xOffset]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }

    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setXOffset:xOffset];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFace:(id <Face>)face yOffset:(int)yOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yOffset:[face yOffset]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setYOffset:yOffset];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)translateFaceOffset:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFaceOffset:face xDelta:-xDelta yDelta:-yDelta];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace translateOffsetsX:xDelta y:yDelta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFace:(id <Face>)face xScale:(float)xScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xScale:[face xScale]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setXScale:xScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFace:(id <Face>)face yScale:(float)yScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yScale:[face yScale]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setYScale:yScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFace:(id <Face>)face rotation:(float)angle {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face rotation:[face rotation]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setRotation:angle];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFace:(id <Face>)face texture:(NSString *)texture {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face texture:[NSString stringWithString:[face texture]]];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:face forKey:FaceKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceWillChange object:self userInfo:userInfo];
    }
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setTexture:texture];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity {
    MutableBrush* brush = [[MutableBrush alloc] init];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    [mutableEntity addBrush:brush];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrush:brush];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brush forKey:BrushKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:userInfo];
        [userInfo release];
    }
    
    return [brush autorelease];
}

- (void)addBrushToEntity:(id <Entity>)theEntity brush:(id <Brush>)theBrush {
    MutableBrush* mutableBrush = (MutableBrush *)theBrush;
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    [mutableEntity addBrush:mutableBrush];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrush:mutableBrush];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:mutableBrush forKey:BrushKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    id <Brush> brush = [[MutableBrush alloc] initWithBrushTemplate:theTemplate];
    [self addBrushToEntity:theEntity brush:brush];
    return [brush autorelease];
}

- (void)deleteBrush:(id <Brush>)brush {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] addBrushToEntity:[brush entity] brush:brush];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brush forKey:BrushKey];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushWillBeRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
    
    MutableEntity* mutableEntity = (MutableEntity *)[brush entity];
    [mutableEntity removeBrush:brush];
    
    if (![mutableEntity isWorldspawn] && [[mutableEntity brushes] count] == 0)
        [self removeEntity:mutableEntity];
}

- (void)translateBrush:(id <Brush>)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBrush:brush xDelta:-xDelta yDelta:-yDelta zDelta:-zDelta];

    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brush forKey:BrushKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushWillChange object:self userInfo:userInfo];
    }
    
    TVector3i delta = {xDelta, yDelta, zDelta};

    MutableBrush* mutableBrush = (MutableBrush *)brush;
    [mutableBrush translateBy:&delta];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateZ90CW:(NSSet *)brushes {
    if ([brushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateZ90CCW:brushes];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    MutableBrush* brush = [brushEn nextObject];
    
    TBoundingBox bounds = *[brush bounds];
    while ((brush = [brushEn nextObject]))
        mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
    
    TVector3f rcf;
    TVector3i rci;
    centerOfBounds(&bounds, &rcf);
    roundV3f(&rcf, &rci);
    
    brushEn = [brushes objectEnumerator];
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        while ((brush = [brushEn nextObject])) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:brush forKey:BrushKey];
            
            [center postNotificationName:BrushWillChange object:self userInfo:userInfo];
            [brush rotateZ90CW:&rci];
            [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
            [userInfo release];
        }
    } else {
        while ((brush = [brushEn nextObject]))
            [brush rotateZ90CW:&rci];
    }
    
}

- (void)rotateZ90CCW:(NSSet *)brushes {
    if ([brushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateZ90CW:brushes];
    
    NSEnumerator* brushEn = [brushes objectEnumerator];
    MutableBrush* brush = [brushEn nextObject];
    
    TBoundingBox bounds = *[brush bounds];
    while ((brush = [brushEn nextObject]))
        mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
    
    TVector3f rcf;
    TVector3i rci;
    centerOfBounds(&bounds, &rcf);
    roundV3f(&rcf, &rci);
    
    brushEn = [brushes objectEnumerator];
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        while ((brush = [brushEn nextObject])) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:brush forKey:BrushKey];
            
            [center postNotificationName:BrushWillChange object:self userInfo:userInfo];
            [brush rotateZ90CCW:&rci];
            [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
            [userInfo release];
        }
    } else {
        while ((brush = [brushEn nextObject]))
            [brush rotateZ90CCW:&rci];
    }
}

- (void)translateFace:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFace:face xDelta:-xDelta yDelta:-yDelta zDelta:-zDelta];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:[face brush] forKey:BrushKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushWillChange object:self userInfo:userInfo];
    }
    
    TVector3i delta = {xDelta, yDelta, zDelta};
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace translateBy:&delta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)dragFace:(id <Face>)face dist:(float)dist {
    MutableFace* mutableFace = (MutableFace *)face;
    if ([mutableFace canDragBy:dist]) {
        NSUndoManager* undoManager = [self undoManager];
        [[undoManager prepareWithInvocationTarget:self] dragFace:face dist:-dist];
        
        NSMutableDictionary* userInfo;
        if ([self postNotifications]) {
            userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:[face brush] forKey:BrushKey];
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:BrushWillChange object:self userInfo:userInfo];
        }
        
        MutableFace* mutableFace = (MutableFace *)face;
        [mutableFace dragBy:(float)dist];
        
        if ([self postNotifications]) {
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
            [userInfo release];
        }
    }
}

- (int)worldSize {
    return worldSize;
}

- (BOOL)postNotifications {
    return postNotifications;
}

- (void)setPostNotifications:(BOOL)value {
    postNotifications = value;
}

- (Picker *)picker {
    return picker;
}

- (GLResources *)glResources {
    return glResources;
}

- (EntityDefinitionManager *)entityDefinitionManager {
    return entityDefinitionManager;
}

- (void)dealloc {
    [entityDefinitionManager release];
    [entities release];
    [picker release];
    [glResources release];
    [super dealloc];
}

@end
