/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#import "MapWriter.h"
#import "EntityRendererManager.h"
#import "SelectionManager.h"
#import "GroupManager.h"
#import "Math.h"

NSString* const FacesWillChange         = @"FacesWillChange";
NSString* const FacesDidChange          = @"FacesDidChange";
NSString* const FacesKey                = @"Faces";

NSString* const BrushesAdded            = @"BrushesAdded";
NSString* const BrushesWillBeRemoved    = @"BrushesWillBeRemoved";
NSString* const BrushesWereRemoved      = @"BrushesWereRemoved";
NSString* const BrushesWillChange       = @"BrushesWillChange";
NSString* const BrushesDidChange        = @"BrushesDidChange";
NSString* const BrushesKey              = @"Brushes";

NSString* const EntitiesAdded           = @"EntitiesAdded";
NSString* const EntitiesWillBeRemoved   = @"EntitiesWillBeRemoved";
NSString* const EntitiesWereRemoved     = @"EntitiesWereRemoved";
NSString* const EntitiesKey             = @"Entities";

NSString* const PropertiesWillChange    = @"PropertiesWillChange";
NSString* const PropertiesDidChange     = @"PropertiesDidChange";

NSString* const PointFileLoaded         = @"PointFileLoaded";
NSString* const PointFileUnloaded       = @"PointFileUnloaded";

NSString* const DocumentCleared         = @"DocumentCleared";
NSString* const DocumentLoaded          = @"DocumentLoaded";

@interface MapDocument (private)

- (void)makeUndoSnapshotOfFaces:(NSArray *)theFaces;
- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofFaces:(NSArray *)theFaces;
- (void)makeUndoSnapshotOfBrushes:(NSArray *)theBrushes;
- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofBrushes:(NSArray *)theBrushes;
- (void)makeUndoSnapshotOfEntities:(NSArray *)theEntities;
- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofEntities:(NSArray *)theEntities;

- (void)postNotification:(NSString *)theNotification forFaces:(NSArray *)theFaces;
- (void)postNotification:(NSString *)theNotification forBrushes:(NSArray *)theBrushes;
- (void)postNotification:(NSString *)theNotification forEntities:(NSArray *)theEntities;

@end

@implementation MapDocument (private)

- (void)makeUndoSnapshotOfFaces:(NSArray *)theFaces {
    NSAssert(theFaces != nil, @"face array must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    NSArray* snapshot = [[NSArray alloc] initWithArray:theFaces copyItems:YES];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:snapshot ofFaces:theFaces];

    [snapshot release];
}

- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofFaces:(NSArray *)theFaces {
    NSAssert(theSnapshot != nil, @"snapshot must not be nil");
    NSAssert(theFaces != nil, @"face array must not be nil");
    NSAssert([theSnapshot count] == [theFaces count], @"snapshot must contain the same number of items as face array");
    
    [self makeUndoSnapshotOfFaces:theFaces];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }

    NSEnumerator* faceEn = [theFaces objectEnumerator];
    NSEnumerator* snapshotEn = [theSnapshot objectEnumerator];
    MutableFace* face;
    MutableFace* snapshot;
    
    while ((face = [faceEn nextObject]) && (snapshot = [snapshotEn nextObject])) {
        NSAssert([face faceId] == [snapshot faceId], @"face and snapshot must have the same id");
        [face restore:snapshot];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)makeUndoSnapshotOfBrushes:(NSArray *)theBrushes {
    NSAssert(theBrushes != nil, @"brush array must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSArray* snapshot = [[NSArray alloc] initWithArray:theBrushes copyItems:YES];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:snapshot ofBrushes:theBrushes];
    
    [snapshot release];
}

- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofBrushes:(NSArray *)theBrushes {
    NSAssert(theSnapshot != nil, @"snapshot must not be nil");
    NSAssert(theBrushes != nil, @"face array must not be nil");
    NSAssert([theSnapshot count] == [theBrushes count], @"snapshot must contain the same number of items as brush array");

    [self makeUndoSnapshotOfBrushes:theBrushes];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }

    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    NSEnumerator* snapshotEn = [theSnapshot objectEnumerator];
    MutableBrush* brush;
    MutableBrush* snapshot;

    while ((brush = [brushEn nextObject]) && (snapshot = [snapshotEn nextObject])) {
        NSAssert([brush brushId] == [snapshot brushId], @"brush and snapshot must have the same id");
        [brush restore:snapshot];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)makeUndoSnapshotOfEntities:(NSArray *)theEntities {
    NSAssert(theEntities != nil, @"enitity array must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSMutableArray* snapshot = [[NSMutableArray alloc] initWithCapacity:[theEntities count]];
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    
    while ((entity = [entityEn nextObject])) {
        NSDictionary* properties = [[NSDictionary alloc] initWithDictionary:[entity properties] copyItems:YES];
        [snapshot addObject:properties];
        [properties release];
    }

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:snapshot ofEntities:theEntities];
    
    [snapshot release];
}


- (void)restoreUndoSnapshot:(NSArray *)theSnapshot ofEntities:(NSArray *)theEntities {
    NSAssert(theSnapshot != nil, @"snapshot must not be nil");
    NSAssert(theEntities != nil, @"entity array must not be nil");
    NSAssert([theSnapshot count] == [theEntities count], @"snapshot must contain the same number of items as entity array");
    
    [self makeUndoSnapshotOfEntities:theEntities];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesWillChange object:self userInfo:userInfo];
    }
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    NSEnumerator* snapshotEn = [theSnapshot objectEnumerator];
    MutableEntity* entity;
    NSDictionary* properties;
    
    while ((entity = [entityEn nextObject]) && (properties = [snapshotEn nextObject]))
        [entity replaceProperties:properties];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)postNotification:(NSString *)theNotification forFaces:(NSArray *)theFaces {
    if ([self postNotifications]) {
        NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theFaces, FacesKey, nil];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:theNotification object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)postNotification:(NSString *)theNotification forBrushes:(NSArray *)theBrushes {
    if ([self postNotifications]) {
        NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theBrushes, BrushesKey, nil];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:theNotification object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)postNotification:(NSString *)theNotification forEntities:(NSArray *)theEntities {
    if ([self postNotifications]) {
        NSDictionary* userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:theEntities, EntitiesKey, nil];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:theNotification object:self userInfo:userInfo];
        [userInfo release];
    }
}

@end

@implementation MapDocument

- (id)init {
    if ((self = [super init])) {
        selectionManager = [[SelectionManager alloc] init];
        
        worldBounds.min.x = -0x1000;
        worldBounds.min.y = -0x1000;
        worldBounds.min.z = -0x1000;
        worldBounds.max.x = +0x1000;
        worldBounds.max.y = +0x1000;
        worldBounds.max.z = +0x1000;
        
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* definitionPath = [mainBundle pathForResource:@"quake" ofType:@"def"];
        entityDefinitionManager = [[EntityDefinitionManager alloc] initWithDefinitionFile:definitionPath];

        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
        postNotifications = YES;

        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];
        glResources = [[GLResources alloc] initWithPalette:palette];
        [palette release];

        picker = [[Picker alloc] initWithMap:self];
        groupManager = [[GroupManager alloc] initWithMap:self];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityDefinitionManager release];
    [groupManager release];
    [entities release];
    [picker release];
    [glResources release];
    [selectionManager release];
    [super dealloc];
}

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapDocumentWindow"];
	[self addWindowController:controller];
    [controller release];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    [self clear];
    
    ProgressWindowController* pwc = [[ProgressWindowController alloc] initWithWindowNibName:@"ProgressWindow"];
    [[pwc window] makeKeyAndOrderFront:self];
    [[pwc label] setStringValue:@"Loading map file..."];
    
    NSProgressIndicator* indicator = [pwc progressIndicator];
    [indicator setIndeterminate:NO];
    [indicator setUsesThreadedAnimation:YES];
    
    [[self undoManager] disableUndoRegistration];
    [self setPostNotifications:NO];
    
    TextureManager* textureManager = [glResources textureManager];
    
    MapParser* parser = [[MapParser alloc] initWithData:data];
    [parser parseMap:self textureManager:textureManager withProgressIndicator:indicator];
    [parser release];

    // set the entity definitions
    for (MutableEntity* entity in entities)
        [self setEntityDefinition:entity];

    [pwc close];
    [pwc release];
    
    NSString* wads = [[self worldspawn:NO] propertyForKey:@"wad"];
    if (wads != nil) {
        TextureManager* textureManager = [glResources textureManager];
        NSArray* wadPaths = [wads componentsSeparatedByString:@";"];
        for (int i = 0; i < [wadPaths count]; i++) {
            NSString* wadPath = [[wadPaths objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            [self insertObject:wadPath inTextureWadsAtIndex:[[textureManager textureCollections] count]];
        }
    }
    
    [self setPostNotifications:YES];
    [[self undoManager] enableUndoRegistration];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:DocumentLoaded object:self];
    
    return YES;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError {
    MapWriter* mapWriter = [[MapWriter alloc] initWithMap:self];
    [mapWriter writeToFileAtUrl:absoluteURL];
    [mapWriter release];
    
    return YES;
}

# pragma mark Point file support

- (void)loadPointFile:(NSData *)theData {
    if (leakPointCount > 0)
        [self unloadPointFile];
    
    NSString* string = [[[NSString alloc] initWithData:theData encoding:NSASCIIStringEncoding] autorelease];
    string = [string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    NSArray* lines = [string componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
    
    if ([lines count] == 0)
        return;
    
    leakPointCount = [lines count];
    leakPoints = malloc(leakPointCount * sizeof(TVector3f));
    int lineIndex = 0;
    
    for (NSString* line in lines) {
        line = [line stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        if (!parseV3f(line, NSMakeRange(0, [line length]), &leakPoints[lineIndex])) {
            free(leakPoints);
            leakPointCount = 0;
            NSLog(@"Error parsing point file at line %i", lineIndex + 1);
            return;
        }
        lineIndex++;
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:PointFileLoaded object:self];
}

- (void)unloadPointFile {
    if (leakPointCount > 0) {
        free(leakPoints);
        leakPoints = NULL;
        leakPointCount = 0;
        
        [[NSNotificationCenter defaultCenter] postNotificationName:PointFileUnloaded object:self];
    }
}

- (TVector3f *)leakPoints {
    return leakPoints;
}

- (int)leakPointCount {
    return leakPointCount;
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
        
        TextureManager* textureManager = [glResources textureManager];
        [textureManager addTextureCollection:collection atIndex:theIndex];
        [collection release];
        
        [self setEntity:[self worldspawn:YES] propertyKey:@"wad" value:[textureManager wadProperty]];
        [self updateFaceTextures];
    } else {
        NSLog(@"wad file '%@' does not exist", theWadPath);
    }
}

- (void)removeObjectFromTextureWadsAtIndex:(NSUInteger)theIndex {
    TextureManager* textureManager = [glResources textureManager];
    [textureManager removeTextureCollectionAtIndex:theIndex];
    
    MutableEntity* wc = [self worldspawn:YES];
    [wc setProperty:@"wad" value:[textureManager wadProperty]];
    
    [self updateFaceTextures];
}

- (NSArray *)textureWads {
    NSMutableArray* textureWads = [[NSMutableArray alloc] init];

    TextureManager* textureManager = [glResources textureManager];
    for (TextureCollection* collection in [textureManager textureCollections])
        [textureWads addObject:[collection name]];
    
    return [textureWads autorelease];
}

- (void)updateFaceTextures {
    TextureManager* textureManager = [glResources textureManager];
    
    NSMutableArray* changedFaces = [[NSMutableArray alloc] init];
    NSMutableArray* newTextures = [[NSMutableArray alloc] init];
    
    for (id <Entity> entity in entities) {
        for (id <Brush> brush in [entity brushes]) {
            for (id <Face> face in [brush faces]) {
                Texture* oldTexture = [face texture];
                Texture* newTexture = [textureManager textureForName:[oldTexture name]];
                if (oldTexture != newTexture) {
                    [changedFaces addObject:face];
                    [newTextures addObject:newTexture];
                }
            }
        }
    }
    
    if ([changedFaces count] > 0) {
        [self postNotification:FacesWillChange forFaces:changedFaces];
        
        NSEnumerator* faceEn = [changedFaces objectEnumerator];
        NSEnumerator* textureEn = [newTextures objectEnumerator];
        MutableFace* face;
        Texture* texture;
        while ((face = [faceEn nextObject]) && (texture = [textureEn nextObject]))
            [face setTexture:texture];
        
        [self postNotification:FacesDidChange forFaces:changedFaces];
    }
    
    [changedFaces release];
    [newTextures release];
}

# pragma mark Map related functions

- (const TBoundingBox *)worldBounds {
    return &worldBounds;
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

- (void)duplicateEntities:(NSArray *)theEntities newEntities:(NSMutableArray *)theNewEntities newBrushes:(NSMutableArray *)theNewBrushes {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    NSAssert(theNewEntities != nil, @"new entitiy set must not be nil");
    NSAssert(theNewBrushes != nil, @"new brush set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    for (id <Entity> entity in theEntities) {
        id <Entity> newEntity = [self createEntityWithProperties:[entity properties]];
        
        if ([[entity entityDefinition] type] != EDT_POINT) {
            for (id <Brush> brush in [entity brushes]) {
                id <Brush> newBrush = [self createBrushInEntity:newEntity fromTemplate:brush];
                [theNewBrushes addObject:newBrush];
            }
        }
        
        [theNewEntities addObject:newEntity];
    }
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

    [self postNotification:PropertiesWillChange forEntities:[NSArray arrayWithObject:theEntity]];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    if (theValue == nil)
        [mutableEntity removeProperty:theKey];
    else
        [mutableEntity setProperty:theKey value:theValue];
    
    [self postNotification:PropertiesDidChange forEntities:[NSArray arrayWithObject:theEntity]];
}

- (void)setEntities:(NSArray *)theEntities propertyKey:(NSString *)theKey value:(NSString *)theValue {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSMutableArray* changedEntities = [[NSMutableArray alloc] init];
    for (id <Entity> entity in theEntities) {
        NSString* oldValue = [entity propertyForKey:theKey];
        if (oldValue == nil) {
            if (theValue != nil)
                [changedEntities addObject:entity];
        } else if (![oldValue isEqualToString:theValue])
            [changedEntities addObject:entity];
    }
    
    if ([changedEntities count] > 0) {
        [self postNotification:PropertiesWillChange forEntities:changedEntities];

        NSUndoManager* undoManager = [self undoManager];

        for (MutableEntity* mutableEntity in changedEntities) {
            [[undoManager prepareWithInvocationTarget:self] setEntity:mutableEntity propertyKey:theKey value:[mutableEntity propertyForKey:theKey]];
            
            if (theValue == nil)
                [mutableEntity removeProperty:theKey];
            else
                [mutableEntity setProperty:theKey value:theValue];
        }

        [self postNotification:PropertiesDidChange forEntities:changedEntities];
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

- (void)translateEntities:(NSArray *)theEntities delta:(TVector3f)theDelta {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;

    TVector3f inverse;
    scaleV3f(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateEntities:[[theEntities copy] autorelease] delta:inverse];

    [self postNotification:PropertiesWillChange forEntities:theEntities];
    for (MutableEntity* entity in theEntities)
        [entity translateBy:&theDelta];
    [self postNotification:PropertiesDidChange forEntities:theEntities];
}

- (void)rotateEntities90CW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntities90CCW:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
    [self postNotification:PropertiesWillChange forEntities:theEntities];
    for (MutableEntity* entity in theEntities)
        [entity rotate90CW:theAxis center:&theCenter];
    [self postNotification:PropertiesDidChange forEntities:theEntities];
}

- (void)rotateEntities90CCW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntities90CW:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
    [self postNotification:PropertiesWillChange forEntities:theEntities];
    for (MutableEntity* entity in theEntities)
        [entity rotate90CCW:theAxis center:&theCenter];
    [self postNotification:PropertiesDidChange forEntities:theEntities];
}

- (void)rotateEntities:(NSArray *)theEntities rotation:(TQuaternion)theRotation center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    if (nullQ(&theRotation))
        return;
    
    [self makeUndoSnapshotOfEntities:theEntities];
    
    [self postNotification:PropertiesWillChange forEntities:theEntities];
    for (MutableEntity* entity in theEntities)
        [entity rotate:&theRotation center:&theCenter];
    [self postNotification:PropertiesDidChange forEntities:theEntities];
}

- (void)flipEntities:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] flipEntities:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
    [self postNotification:PropertiesWillChange forEntities:theEntities];
    for (MutableEntity* entity in theEntities)
        [entity flipAxis:theAxis center:&theCenter];
    [self postNotification:PropertiesDidChange forEntities:theEntities];
}

- (void)deleteEntities:(NSArray *)theEntities {
    [self removeEntities:theEntities];
}

# pragma mark Brush related functions

- (void)addBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrushes:[[theBrushes copy] autorelease]];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    for (MutableBrush* brush in theBrushes)
        [mutableEntity addBrush:brush];
    [self postNotification:BrushesAdded forBrushes:theBrushes];
}

- (void)moveBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];

    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    NSMutableDictionary* entityBrushes = [[NSMutableDictionary alloc] init];
    NSMutableArray* changedEntities = [[NSMutableArray alloc] init];
    
    for (MutableBrush* brush in theBrushes) {
        MutableEntity* previousEntity = [brush entity];
        NSMutableArray* brushArray = [entityBrushes objectForKey:[previousEntity entityId]];
        if (brushArray == nil) {
            brushArray = [[NSMutableArray alloc] init];
            [entityBrushes setObject:brushArray forKey:[previousEntity entityId]];
            [brushArray release];
            [changedEntities addObject:previousEntity];
        }
        
        [brushArray addObject:brush];
        [previousEntity removeBrush:brush];
        [mutableEntity addBrush:brush];
    }
    
    NSUndoManager* undoManager = [self undoManager];
    for (id <Entity> changedEntity in changedEntities) {
        NSArray* brushArray = [entityBrushes objectForKey:[changedEntity entityId]];
        [[undoManager prepareWithInvocationTarget:self] moveBrushesToEntity:changedEntity brushes:brushArray];
    }
    
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theTemplate != nil, @"brush template must not be nil");
    
    if (!boundsContainBounds(&worldBounds, [theTemplate bounds])) {
        NSLog(@"brush template is not within world bounds");
        return nil;
    }
    
    id <Brush> brush = [[MutableBrush alloc] initWithWorldBounds:&worldBounds brushTemplate:theTemplate];
    [self addBrushesToEntity:theEntity brushes:[NSArray arrayWithObject:brush]];
    return [brush autorelease];
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity withBounds:(TBoundingBox *)theBounds texture:(Texture *)theTexture {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theBounds != NULL, @"brush bounds must not be NULL");
    NSAssert(theTexture != nil, @"brush texture must not be nil");
    
    if (!boundsContainBounds(&worldBounds, theBounds)) {
        NSLog(@"bounds are not within world bounds");
        return nil;
    }
    
    id <Brush> brush = [[MutableBrush alloc] initWithWorldBounds:&worldBounds brushBounds:theBounds texture:theTexture];
    [self addBrushesToEntity:theEntity brushes:[NSArray arrayWithObject:brush]];
    return [brush autorelease];
}

- (void)duplicateBrushes:(NSArray *)theBrushes newBrushes:(NSMutableArray *)theNewBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSAssert(theNewBrushes != nil, @"new brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    for (id <Brush> brush in theBrushes) {
        id <Brush> newBrush = [self createBrushInEntity:[self worldspawn:YES] fromTemplate:brush];
        [theNewBrushes addObject:newBrush];
    }
}

- (void)translateBrushes:(NSArray *)theBrushes delta:(TVector3f)theDelta lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    TVector3f inverse;
    scaleV3f(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBrushes:[[theBrushes copy] autorelease] delta:inverse lockTextures:lockTextures];
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush translateBy:&theDelta lockTextures:lockTextures];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)rotateBrushes90CW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushes90CCW:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush rotate90CW:theAxis center:&theCenter lockTextures:lockTextures];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)rotateBrushes90CCW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushes90CW:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush rotate90CCW:theAxis center:&theCenter lockTextures:lockTextures];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)rotateBrushes:(NSArray *)theBrushes rotation:(TQuaternion)theRotation center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    if (nullQ(&theRotation))
        return;

    [self makeUndoSnapshotOfBrushes:[[theBrushes copy] autorelease]];

    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush rotate:&theRotation center:&theCenter lockTextures:lockTextures];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)flipBrushes:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] flipBrushes:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush flipAxis:theAxis center:&theCenter lockTextures:lockTextures];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)snapBrushes:(NSArray *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    [self makeUndoSnapshotOfBrushes:theBrushes];
    
    [self postNotification:BrushesWillChange forBrushes:theBrushes];
    for (MutableBrush* brush in theBrushes)
        [brush snap];
    [self postNotification:BrushesDidChange forBrushes:theBrushes];
}

- (void)deleteBrushes:(NSArray *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableArray* changedEntities = [[NSMutableArray alloc] init];
    NSMutableArray* emptyEntities = [[NSMutableArray alloc] init];
    NSMutableDictionary* entityIdToBrushSet = [[NSMutableDictionary alloc] init];
    
    for (MutableBrush* brush in theBrushes) {
        id <Entity> entity = [brush entity];
        NSMutableArray* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        if (entityBrushes == nil) {
            entityBrushes = [[NSMutableArray alloc] init];
            [entityIdToBrushSet setObject:entityBrushes forKey:[entity entityId]];
            [entityBrushes release];
            [changedEntities addObject:entity];
        }
        [entityBrushes addObject:brush];
    }
    
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    for (MutableEntity* entity in changedEntities) {
        NSArray* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        [selectionManager removeBrushes:entityBrushes record:YES];
        [[undoManager prepareWithInvocationTarget:self] addBrushesToEntity:entity brushes:[[entityBrushes copy] autorelease]];

        [self postNotification:BrushesWillBeRemoved forBrushes:entityBrushes];
        for (id <Brush> brush in entityBrushes)
            [entity removeBrush:brush];
        [self postNotification:BrushesWereRemoved forBrushes:entityBrushes];
        
        if (![entity isWorldspawn] && [[entity brushes] count] == 0)
            [emptyEntities addObject:entity];
    }
    
    [self removeEntities:emptyEntities];
    [emptyEntities release];
    [changedEntities release];
    [entityIdToBrushSet release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Delete Brushes"];
}

# pragma mark Face related functions

- (void)setFaces:(NSArray *)theFaces xOffset:(int)theXOffset {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setXOffset:theXOffset];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)setFaces:(NSArray *)theFaces yOffset:(int)theYOffset {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setYOffset:theYOffset];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)translateFaceOffsets:(NSArray *)theFaces delta:(float)theDelta dir:(TVector3f)theDir {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0 || theDelta == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFaceOffsets:[[theFaces copy] autorelease] delta:-theDelta dir:theDir];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face translateOffsetsBy:theDelta dir:&theDir];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)setFaces:(NSArray *)theFaces xScale:(float)theXScale {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];

    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setXScale:theXScale];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)setFaces:(NSArray *)theFaces yScale:(float)theYScale {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setYScale:theYScale];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)scaleFaces:(NSArray *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    if (theXFactor == 0 && theYFactor == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] scaleFaces:[[theFaces copy] autorelease] xFactor:-theXFactor yFactor:-theYFactor];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces) {
        [face setXScale:theXFactor + [face xScale]];
        [face setYScale:theYFactor + [face yScale]];
    }
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)setFaces:(NSArray *)theFaces rotation:(float)theAngle {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setRotation:theAngle];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)rotateFaces:(NSArray *)theFaces angle:(float)theAngle {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    if (theAngle == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateFaces:[[theFaces copy] autorelease] angle:-theAngle];

    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setRotation:theAngle + [face rotation]];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (void)setFaces:(NSArray *)theFaces texture:(Texture *)theTexture {
    NSAssert(theFaces != nil, @"face set must not be nil");
    NSAssert(theTexture != nil, @"texture must not be nil");
    
    if ([theFaces count] == 0)
        return;
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];

    [self postNotification:FacesWillChange forFaces:theFaces];
    for (MutableFace* face in theFaces)
        [face setTexture:theTexture];
    [self postNotification:FacesDidChange forFaces:theFaces];
}

- (BOOL)dragFaces:(NSArray *)theFaces distance:(float)theDistance lockTextures:(BOOL)lockTextures {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return NO;
    
    if (theDistance == 0)
        return NO;
    
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    BOOL canDrag = YES;
    for (id <Face> face in theFaces) {
        MutableBrush* brush = [face brush];
        [brushes addObject:brush];
        canDrag &= [brush canDrag:face by:theDistance];
    }

    if (canDrag) {
        NSUndoManager* undoManager = [self undoManager];
        [[undoManager prepareWithInvocationTarget:self] dragFaces:[[theFaces copy] autorelease] distance:-theDistance lockTextures:lockTextures];

        [self postNotification:BrushesWillChange forBrushes:brushes];
        for (MutableFace* face in theFaces) {
            MutableBrush* brush = [face brush];
            [brush drag:face by:theDistance lockTexture:lockTextures];
        }
        [self postNotification:BrushesDidChange forBrushes:brushes];
    }

    [brushes release];
    return canDrag;
}

- (TDragResult)dragVertex:(int)theVertexIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    MutableBrush* mutableBrush = (MutableBrush *)theBrush;
    NSArray* brushArray = [[NSArray alloc] initWithObjects:theBrush, nil];

    [self makeUndoSnapshotOfBrushes:brushArray];
    [self postNotification:BrushesWillChange forBrushes:brushArray];
    TDragResult result = [mutableBrush dragVertex:theVertexIndex by:theDelta];
    [self postNotification:BrushesDidChange forBrushes:brushArray];
    
    [brushArray release];
    return result;
}

- (TDragResult)dragEdge:(int)theEdgeIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    MutableBrush* mutableBrush = (MutableBrush *)theBrush;
    NSArray* brushArray = [[NSArray alloc] initWithObjects:theBrush, nil];
    
    [self makeUndoSnapshotOfBrushes:brushArray];
    [self postNotification:BrushesWillChange forBrushes:brushArray];
    TDragResult result = [mutableBrush dragEdge:theEdgeIndex by:theDelta];
    [self postNotification:BrushesDidChange forBrushes:brushArray];
    
    [brushArray release];
    return result;
}

- (TDragResult)dragFace:(int)theFaceIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    MutableBrush* mutableBrush = (MutableBrush *)theBrush;
    NSArray* brushArray = [[NSArray alloc] initWithObjects:theBrush, nil];
    
    [self makeUndoSnapshotOfBrushes:brushArray];
    [self postNotification:BrushesWillChange forBrushes:brushArray];
    TDragResult result = [mutableBrush dragFace:theFaceIndex by:theDelta];
    [self postNotification:BrushesDidChange forBrushes:brushArray];
    
    [brushArray release];
    return result;
}

- (void)clear {
    [selectionManager removeAll:NO];
    [self unloadPointFile];
    [entities removeAllObjects];
    worldspawn = nil;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:DocumentCleared object:self];

    [[self undoManager] removeAllActions];
    [glResources reset];
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

- (SelectionManager *)selectionManager {
    return selectionManager;
}

- (GroupManager *)groupManager {
    return groupManager;
}

# pragma mark @implementation Map

- (void)addEntities:(NSArray *)theEntities {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    [[[self undoManager] prepareWithInvocationTarget:self] removeEntities:[[theEntities copy] autorelease]];

    for (MutableEntity* entity in theEntities) {
        if (![entity isWorldspawn] || [self worldspawn:NO] == nil) {
            [entities addObject:entity];
            [entity setMap:self];
        }
    }
    
    [self postNotification:EntitiesAdded forEntities:theEntities];
}

- (void)addEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    NSArray* entityArray = [[NSArray alloc] initWithObjects:theEntity, nil];
    [self addEntities:entityArray];
    [entityArray release];
}

- (void)removeEntities:(NSArray *)theEntities {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;

    [[self selectionManager] removeEntities:theEntities record:YES];
    [[[self undoManager] prepareWithInvocationTarget:self] addEntities:[[theEntities copy] autorelease]];
    
    [self postNotification:EntitiesWillBeRemoved forEntities:theEntities];
    for (MutableEntity* entity in theEntities) {
        [entity setMap:nil];
        [entities removeObject:entity];
        if (worldspawn == entity)
            worldspawn = nil;
    }
    [self postNotification:EntitiesWereRemoved forEntities:theEntities];
}

- (void)removeEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    NSArray* entityArray = [[NSArray alloc] initWithObjects:theEntity, nil];
    [self removeEntities:entityArray];
    [entityArray release];
}

- (id <Entity>)worldspawn:(BOOL)create {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        for (worldspawn in entities)
            if ([worldspawn isWorldspawn])
                break;
    }
    
    if (worldspawn == nil && create) {
        EntityDefinition* entityDefinition = [entityDefinitionManager definitionForName:WorldspawnClassname];
        
        MutableEntity* entity = [[MutableEntity alloc] init];
        [entity setProperty:ClassnameKey value:WorldspawnClassname];
        [entity setEntityDefinition:entityDefinition];
        [entities addObject:entity];
        [entity setMap:self];

        [self postNotification:EntitiesAdded forEntities:[NSArray arrayWithObject:entity]];
        
        worldspawn = entity;
        [entity release];
    }
    
    return worldspawn;
}

- (NSArray *)entities {
    return entities;
}

@end
