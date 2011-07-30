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
#import "FaceInfo.h"
#import "BrushInfo.h"
#import "EntityInfo.h"
#import "TextureManager.h"
#import "TextureCollection.h"
#import "Texture.h"
#import "Picker.h"
#import "GLResources.h"
#import "WadLoader.h"
#import "MapWindowController.h"
#import "ProgressWindowController.h"
#import "MapParser.h"
#import "MapWriter.h"
#import "EntityRendererManager.h"

NSString* const FacesWillChange         = @"FacesWillChange";
NSString* const FacesDidChange          = @"FacesDidChange";
NSString* const FacesKey                = @"Faces";

NSString* const BrushesAdded            = @"BrushesAdded";
NSString* const BrushesWillBeRemoved    = @"BrushesWillBeRemoved";
NSString* const BrushesWillChange       = @"BrushesWillChange";
NSString* const BrushesDidChange        = @"BrushesDidChange";
NSString* const BrushesKey              = @"Brushes";

NSString* const EntitiesAdded           = @"EntitiesAdded";
NSString* const EntitiesWillBeRemoved   = @"EntitiesWillBeRemoved";
NSString* const EntitiesKey             = @"Entities";

NSString* const PropertiesWillChange    = @"PropertiesWillChange";
NSString* const PropertiesDidChange     = @"PropertiesDidChange";

@interface MapDocument (private)

- (void)makeUndoSnapshot:(NSSet *)theFaces;
- (void)restoreUndoSnapshot:(NSSet *)theFaces faceInfos:(NSDictionary *)theFaceInfos;
- (void)restoreUndoSnapshot:(NSSet *)theBrushes brushInfos:(NSDictionary *)theBrushInfos;
- (void)restoreUndoSnapshot:(NSSet *)theEntities entityInfos:(NSDictionary *)theEntityInfos;

@end

@implementation MapDocument (private)

- (void)makeUndoSnapshot:(NSSet *)theFaces {
    NSMutableDictionary* faceInfos = [[NSMutableDictionary alloc] init];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        FaceInfo* faceInfo = [[FaceInfo alloc] initWithFace:face];
        [faceInfos setObject:faceInfo forKey:[face faceId]];
        [faceInfo release];
    }
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theFaces faceInfos:faceInfos];

    [faceInfos release];
}

- (void)restoreUndoSnapshot:(NSSet *)theFaces faceInfos:(NSDictionary *)theFaceInfos {
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject])) {
        FaceInfo* faceInfo = [theFaceInfos objectForKey:[face faceId]];
        [faceInfo updateFace:face];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)restoreUndoSnapshot:(NSSet *)theBrushes brushInfos:(NSDictionary *)theBrushInfos {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSAssert(theBrushInfos != nil, @"brush info dictionary must not be nil");
    NSAssert([theBrushes count] == [theBrushInfos count], @"brush set must be of the same size as brush info dictionary");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }
    
    NSMutableDictionary* undoInfos = [[NSMutableDictionary alloc] initWithCapacity:[theBrushInfos count]];
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject])) {
        BrushInfo* undoInfo = [[BrushInfo alloc] initWithBrush:brush];
        [undoInfos setObject:undoInfo forKey:[brush brushId]];
        
        BrushInfo* brushInfo = [theBrushInfos objectForKey:[brush brushId]];
        [brushInfo updateBrush:brush];
    }
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theBrushes brushInfos:undoInfos];
    [undoInfos release];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)restoreUndoSnapshot:(NSSet *)theEntities entityInfos:(NSDictionary *)theEntityInfos {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    NSAssert(theEntityInfos != nil, @"entity info dictionary must not be nil");
    NSAssert([theEntities count] == [theEntityInfos count], @"entity set must be of the same size as entity info dictionary");
    
    if ([theEntities count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    NSMutableDictionary* undoInfos = [[NSMutableDictionary alloc] initWithCapacity:[theEntityInfos count]];
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        EntityInfo* undoInfo = [[EntityInfo alloc] initWithEntity:entity];
        [undoInfos setObject:undoInfo forKey:[entity entityId]];
        
        EntityInfo* entityInfo = [theEntityInfos objectForKey:[entity entityId]];
        [entityInfo updateEntity:entity];
    }
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theEntities entityInfos:undoInfos];
    [undoInfos release];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

@end

@implementation MapDocument

- (id)init {
    if ((self = [super init])) {
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* definitionPath = [mainBundle pathForResource:@"quake" ofType:@"def"];
        entityDefinitionManager = [[EntityDefinitionManager alloc] initWithDefinitionFile:definitionPath];

        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
        worldSize = 8192;
        postNotifications = YES;

        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];
        glResources = [[GLResources alloc] initWithPalette:palette];

        picker = [[Picker alloc] initWithDocument:self];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityDefinitionManager release];
    [entities release];
    [picker release];
    [glResources release];
    [super dealloc];
}

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapDocument"];
	[self addWindowController:controller];
    [controller release];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
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

    NSString* wads = [[self worldspawn:NO] propertyForKey:@"wad"];
    if (wads != nil) {
        TextureManager* textureManager = [glResources textureManager];
        NSArray* wadPaths = [wads componentsSeparatedByString:@";"];
        for (int i = 0; i < [wadPaths count]; i++) {
            NSString* wadPath = [[wadPaths objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            [self insertObject:wadPath inTextureWadsAtIndex:[[textureManager textureCollections] count]];
        }
    }
    
    return YES;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    MapWriter* mapWriter = [[MapWriter alloc] initWithMap:self];
    [mapWriter writeToFileAtUrl:absoluteURL];
    [mapWriter release];
    
    return YES;
}

# pragma mark Texture wad management
- (void)insertObject:(NSString *)theWadPath inTextureWadsAtIndex:(NSUInteger)theIndex {
    NSAssert(theWadPath != nil, @"wad path must not be nil");
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:theWadPath]) {
        int slashIndex = [theWadPath rangeOfString:@"/" options:NSBackwardsSearch].location;
        NSString* wadName = [theWadPath substringFromIndex:slashIndex + 1];
        
        WadLoader* wadLoader = [[WadLoader alloc] init];
        Wad* wad = [wadLoader loadFromData:[NSData dataWithContentsOfMappedFile:theWadPath] wadName:wadName];
        [wadLoader release];
        
        NSData* palette = [glResources palette];
        
        TextureCollection* collection = [[TextureCollection alloc] initName:theWadPath palette:palette wad:wad];
        [palette release];
        
        TextureManager* textureManager = [glResources textureManager];
        [textureManager addTextureCollection:collection atIndex:theIndex];
        [collection release];
        
        [self setEntity:[self worldspawn:YES] propertyKey:@"wad" value:[textureManager wadProperty]];
        [self updateTextureUsageCounts];
    } else {
        NSLog(@"wad file '%@' does not exist", theWadPath);
    }
}

- (void)removeObjectFromTextureWadsAtIndex:(NSUInteger)theIndex {
    TextureManager* textureManager = [glResources textureManager];
    [textureManager removeTextureCollectionAtIndex:theIndex];
    
    MutableEntity* wc = [self worldspawn:YES];
    [wc setProperty:@"wad" value:[textureManager wadProperty]];
    
    [self updateTextureUsageCounts];
}

- (NSArray *)textureWads {
    NSMutableArray* textureWads = [[NSMutableArray alloc] init];

    TextureManager* textureManager = [glResources textureManager];
    NSEnumerator* collectionEn = [[textureManager textureCollections] objectEnumerator];
    TextureCollection* collection;
    while ((collection = [collectionEn nextObject]))
        [textureWads addObject:[collection name]];
    
    return [textureWads autorelease];
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

# pragma mark Map related functions

- (int)worldSize {
    return worldSize;
}

- (BOOL)postNotifications {
    return postNotifications;
}

- (void)setPostNotifications:(BOOL)value {
    postNotifications = value;
}

# pragma mark Entity related functions

- (id <Entity>)createEntityWithClassname:(NSString *)classname {
    NSAssert(classname != nil, @"class name must not be nil");
    
    EntityDefinition* entityDefinition = [entityDefinitionManager definitionForName:classname];
    if (entityDefinition == nil) {
        NSLog(@"No entity definition for class name '%@'", classname);
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
        NSLog(@"No entity definition for class name '%@'", classname);
        return nil;
    }
    
    MutableEntity* entity = [[MutableEntity alloc] initWithProperties:properties];
    [entity setEntityDefinition:entityDefinition];
    
    [self addEntity:entity];
    return [entity autorelease];
}

- (void)setEntity:(id <Entity>)theEntity propertyKey:(NSString *)theKey value:(NSString *)theValue {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    
    NSString* oldValue = [theEntity propertyForKey:theKey];
    
    if (oldValue == nil) {
        if (theValue == nil)
            return;
    } else if ([oldValue isEqualToString:theValue])
        return;
    
    [[[self undoManager] prepareWithInvocationTarget:self] setEntity:theEntity propertyKey:theKey value:oldValue];
    
    NSMutableDictionary* userInfo = nil;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        NSSet* userInfoEntities = [[NSSet alloc] initWithObjects:theEntity, nil];
        
        [userInfo setObject:userInfoEntities forKey:EntitiesKey];
        [userInfoEntities release];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    if (theValue == nil)
        [mutableEntity removeProperty:theKey];
    else
        [mutableEntity setProperty:theKey value:theValue];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setEntities:(NSSet *)theEntities propertyKey:(NSString *)theKey value:(NSString *)theValue {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSMutableSet* changedEntities = [[NSMutableSet alloc] init];
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    
    while ((entity = [entityEn nextObject])) {
        NSString* oldValue = [entity propertyForKey:theKey];
        if (oldValue == nil) {
            if (theValue != nil)
                [changedEntities addObject:entity];
        } else if (![oldValue isEqualToString:theValue])
            [changedEntities addObject:entity];
    }
    
    
    if ([changedEntities count] > 0) {
        NSMutableDictionary* userInfo = nil;
        if ([self postNotifications]) {
            userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:changedEntities forKey:EntitiesKey];
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
        }

        NSUndoManager* undoManager = [self undoManager];

        NSEnumerator* changedEntityEn = [changedEntities objectEnumerator];
        MutableEntity* mutableEntity;
        while ((mutableEntity = [changedEntityEn nextObject])) {
            [[undoManager prepareWithInvocationTarget:self] setEntity:mutableEntity propertyKey:theKey value:[mutableEntity propertyForKey:theKey]];
            
            if (theValue == nil)
                [mutableEntity removeProperty:theKey];
            else
                [mutableEntity setProperty:theKey value:theValue];
        }

        if ([self postNotifications]) {
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
            [userInfo release];
        }
    }
    
    [changedEntities release];
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

- (void)translateEntities:(NSSet *)theEntities delta:(TVector3i)theDelta {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    TVector3i inverse;
    scaleV3i(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateEntities:theEntities delta:inverse];

    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }

    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject]))
        [entity translateBy:&theDelta];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)translateEntities:(NSSet *)theEntities direction:(const TVector3f)theDirection delta:(int)theDelta {
    
    TVector3f a;
    TVector3i d;
    closestAxisV3f(&theDirection, &a);
    scaleV3f(&a, theDelta, &a);
    roundV3f(&a, &d);
    
    [self translateEntities:theEntities delta:d];
}

- (void)rotateEntitiesZ90CW:(NSSet *)theEntities center:(TVector3i)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntitiesZ90CCW:theEntities center:theCenter];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject]))
        [entity rotateZ90CW:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateEntitiesZ90CCW:(NSSet *)theEntities center:(TVector3i)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntitiesZ90CW:theEntities center:theCenter];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject]))
        [entity rotateZ90CCW:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateEntities:(NSSet *)theEntities rotation:(TQuaternion)theRotation center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    NSMutableDictionary* undoInfos = [[NSMutableDictionary alloc] initWithCapacity:[theEntities count]];
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        EntityInfo* entityInfo = [[EntityInfo alloc] initWithEntity:entity];
        [undoInfos setObject:entityInfo forKey:[entity entityId]];
        [entityInfo release];
        [entity rotate:&theRotation center:&theCenter];
    }
    
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theEntities entityInfos:undoInfos];
    [undoInfos release];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [undoManager endUndoGrouping];
}

- (void)deleteEntities:(NSSet *)theEntities {
    [self removeEntities:theEntities];
}

# pragma mark Brush related functions

- (void)addBrushesToEntity:(id <Entity>)theEntity brushes:(NSSet *)theBrushes {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrushes:theBrushes];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [mutableEntity addBrush:brush];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesAdded object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theTemplate != nil, @"brush template must not be nil");
    
    id <Brush> brush = [[MutableBrush alloc] initWithBrushTemplate:theTemplate];
    
    NSSet* brushSet = [[NSSet alloc] initWithObjects:brush, nil];
    [self addBrushesToEntity:theEntity brushes:brushSet];
    [brushSet release];
    
    return [brush autorelease];
}

- (void)translateBrushes:(NSSet *)theBrushes delta:(TVector3i)theDelta {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    TVector3i inverse;
    scaleV3i(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBrushes:theBrushes delta:inverse];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }

    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [brush translateBy:&theDelta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)translateBrushes:(NSSet *)theBrushes direction:(TVector3f)theDirection delta:(int)theDelta {
    TVector3f a;
    TVector3i d;
    closestAxisV3f(&theDirection, &a);
    scaleV3f(&a, theDelta, &a);
    roundV3f(&a, &d);
    
    [self translateBrushes:theBrushes delta:d];
}

- (void)rotateBrushesZ90CW:(NSSet *)theBrushes center:(TVector3i)theCenter {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushesZ90CCW:theBrushes center:theCenter];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [brush rotateZ90CW:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateBrushesZ90CCW:(NSSet *)theBrushes center:(TVector3i)theCenter {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushesZ90CW:theBrushes center:theCenter];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [brush rotateZ90CCW:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateBrushes:(NSSet *)theBrushes rotation:(TQuaternion)theRotation center:(TVector3f)theCenter {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }
    
    NSMutableDictionary* undoInfos = [[NSMutableDictionary alloc] initWithCapacity:[theBrushes count]];
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject])) {
        BrushInfo* brushInfo = [[BrushInfo alloc] initWithBrush:brush];
        [undoInfos setObject:brushInfo forKey:[brush brushId]];
        [brushInfo release];
        [brush rotate:&theRotation center:&theCenter];
    }
    
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theBrushes brushInfos:undoInfos];
    [undoInfos release];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [undoManager endUndoGrouping];
}

- (void)deleteBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableSet* affectedEntities = [[NSMutableSet alloc] init];
    NSMutableDictionary* entityIdToBrushSet = [[NSMutableDictionary alloc] init];
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> entity = [brush entity];
        NSMutableSet* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        if (entityBrushes == nil) {
            entityBrushes = [[NSMutableSet alloc] init];
            [entityIdToBrushSet setObject:entityBrushes forKey:[entity entityId]];
            [entityBrushes release];
            
            [affectedEntities addObject:entity];
        }
        
        [entityBrushes addObject:brush];
    }
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    NSMutableSet* emptyEntities = [[NSMutableSet alloc] init];
    
    NSEnumerator* entityEn = [affectedEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        NSSet* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        [[undoManager prepareWithInvocationTarget:self] addBrushesToEntity:entity brushes:entityBrushes];

        if ([self postNotifications]) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:entityBrushes forKey:BrushesKey];
            [center postNotificationName:BrushesWillBeRemoved object:self userInfo:userInfo];
            [userInfo release];
        }
        
        NSEnumerator* brushEn = [entityBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject]))
            [entity removeBrush:brush];
        
        if (![entity isWorldspawn] && [[entity brushes] count] == 0)
            [emptyEntities addObject:entity];
    }
    
    [self removeEntities:emptyEntities];
    [emptyEntities release];
    [affectedEntities release];
    [entityIdToBrushSet release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Delete Brushes"];
}

# pragma mark Face related functions

- (void)setFaces:(NSSet *)theFaces xOffset:(int)theXOffset {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }

    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setXOffset:theXOffset];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFaces:(NSSet *)theFaces yOffset:(int)theYOffset {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setYOffset:theYOffset];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)translateFaceOffsets:(NSSet *)theFaces xDelta:(int)theXDelta yDelta:(int)theYDelta {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFaceOffsets:theFaces xDelta:-theXDelta yDelta:-theYDelta];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face translateOffsetsX:theXDelta y:theYDelta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFaces:(NSSet *)theFaces xScale:(float)theXScale {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setXScale:theXScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFaces:(NSSet *)theFaces yScale:(float)theYScale {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setYScale:theYScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)scaleFaces:(NSSet *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    if (theXFactor == 0 && theYFactor == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject])) {
        [face setXScale:theXFactor + [face xScale]];
        [face setYScale:theYFactor + [face yScale]];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFaces:(NSSet *)theFaces rotation:(float)theAngle {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:theAngle];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateFaces:(NSSet *)theFaces angle:(float)theAngle {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    if (theAngle == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:[face rotation] + theAngle];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)setFaces:(NSSet *)theFaces texture:(NSString *)theTexture {
    NSAssert(theFaces != nil, @"face set must not be nil");
    NSAssert(theTexture != nil, @"texture must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshot:theFaces];
    
    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face setTexture:theTexture];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}


- (void)translateFaces:(NSSet *)theFaces delta:(TVector3i)theDelta {
    NSAssert(theFaces != nil, @"face set must not be nil");

    if ([theFaces count] == 0)
        return;
    
    if (theDelta.x == 0 && theDelta.y == 0 && theDelta.z == 0)
        return;
    
    TVector3i inverse;
    scaleV3i(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFaces:theFaces delta:inverse];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        NSMutableSet* brushes = [[NSMutableSet alloc] init];
        NSEnumerator* faceEn = [theFaces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject]))
            [brushes addObject:[face brush]];
        
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brushes forKey:BrushesKey];
        [brushes release];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }

    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face translateBy:&theDelta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)dragFaces:(NSSet *)theFaces distance:(float)theDistance {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    if (theDistance == 0)
        return;
    
    BOOL canDrag = YES;
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        NSMutableSet* brushes = [[NSMutableSet alloc] init];
        NSEnumerator* faceEn = [theFaces objectEnumerator];
        MutableFace* face;
        while ((face = [faceEn nextObject]) && canDrag) {
            [brushes addObject:[face brush]];
            canDrag &= [face canDragBy:theDistance];
        }
        
        if (canDrag) {
            userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:brushes forKey:BrushesKey];
            [brushes release];
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
        }
    } else {
        NSEnumerator* faceEn = [theFaces objectEnumerator];
        MutableFace* face;
        while ((face = [faceEn nextObject]) && canDrag)
            canDrag &= [face canDragBy:theDistance];
    }

    if (!canDrag)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] dragFaces:theFaces distance:-theDistance];

    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face dragBy:theDistance];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

# pragma mark Getters

- (Picker *)picker {
    return picker;
}

- (GLResources *)glResources {
    return glResources;
}

- (EntityDefinitionManager *)entityDefinitionManager {
    return entityDefinitionManager;
}

# pragma mark @implementation Map

- (void)addEntities:(NSSet *)theEntities {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    [[[self undoManager] prepareWithInvocationTarget:self] removeEntities:theEntities];

    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        if (![entity isWorldspawn] || [self worldspawn:NO] == nil) {
            [entities addObject:entity];
            [entity setMap:self];
        }
    }
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntitiesAdded object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)addEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    NSSet* entitySet = [[NSSet alloc] initWithObjects:theEntity, nil];
    [self addEntities:entitySet];
    [entitySet release];
}

- (void)removeEntities:(NSSet *)theEntities {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;

    [[[self undoManager] prepareWithInvocationTarget:self] addEntities:theEntities];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntitiesWillBeRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        [entity setMap:nil];
        [entities removeObject:entity];
        if (worldspawn == entity)
            worldspawn = nil;
    }
}

- (void)removeEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    NSSet* entitySet = [[NSSet alloc] initWithObjects:theEntity, nil];
    [self removeEntities:entitySet];
    [entitySet release];
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

- (NSArray *)entities {
    return entities;
}

@end
