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

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] restoreUndoSnapshot:theBrushes ofBrushes:theSnapshot];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillBeRemoved object:self userInfo:userInfo];
    }

    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    NSEnumerator* snapshotEn = [theSnapshot objectEnumerator];
    MutableBrush* brush;
    MutableBrush* snapshot;

    while ((brush = [brushEn nextObject]) && (snapshot = [snapshotEn nextObject])) {
        NSAssert([brush brushId] == [snapshot brushId], @"brush and snapshot must have the same id");

        MutableEntity* entity = [brush entity];
        [entity addBrush:snapshot];
        
        if ([selectionManager isBrushSelected:brush]) {
            [selectionManager removeBrush:brush record:NO];
            [selectionManager addBrush:snapshot record:NO];
        }
        
        [entity removeBrush:brush];
        [brush setEntity:entity];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWereRemoved object:self userInfo:userInfo];
        [userInfo release];
        
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theSnapshot forKey:BrushesKey];
        [center postNotificationName:BrushesAdded object:self userInfo:userInfo];
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
    NSEnumerator* entityEn = [entities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject]))
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
    
    NSEnumerator* lineEn = [lines objectEnumerator];
    NSString* line;
    while ((line = [lineEn nextObject])) {
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
    NSEnumerator* collectionEn = [[textureManager textureCollections] objectEnumerator];
    TextureCollection* collection;
    while ((collection = [collectionEn nextObject]))
        [textureWads addObject:[collection name]];
    
    return [textureWads autorelease];
}

- (void)updateFaceTextures {
    TextureManager* textureManager = [glResources textureManager];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            MutableFace* face;
            while ((face = [faceEn nextObject])) {
                Texture* oldTexture = [face texture];
                Texture* newTexture = [textureManager textureForName:[oldTexture name]];
                [face setTexture:newTexture];
            }
        }
    }
}

# pragma mark Map related functions

- (TBoundingBox *)worldBounds {
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
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        id <Entity> newEntity = [self createEntityWithProperties:[entity properties]];
        
        if ([[entity entityDefinition] type] != EDT_POINT) {
            NSArray* brushes = [entity brushes];
            if ([brushes count] > 0) {
                NSEnumerator* brushEn = [brushes objectEnumerator];
                id <Brush> brush;
                while ((brush = [brushEn nextObject])) {
                    id <Brush> newBrush = [self createBrushInEntity:newEntity fromTemplate:brush];
                    [theNewBrushes addObject:newBrush];
                }
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
    
    NSMutableDictionary* userInfo = nil;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        NSArray* userInfoEntities = [[NSArray alloc] initWithObjects:theEntity, nil];
        
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

- (void)setEntities:(NSArray *)theEntities propertyKey:(NSString *)theKey value:(NSString *)theValue {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSMutableArray* changedEntities = [[NSMutableArray alloc] init];
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

- (void)translateEntities:(NSArray *)theEntities delta:(TVector3i)theDelta {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;

    TVector3i inverse;
    scaleV3i(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateEntities:[[theEntities copy] autorelease] delta:inverse];

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

- (void)rotateEntities90CW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntities90CCW:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
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
        [entity rotate90CW:theAxis center:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateEntities90CCW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");

    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateEntities90CW:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
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
        [entity rotate90CCW:theAxis center:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateEntities:(NSArray *)theEntities rotation:(TQuaternion)theRotation center:(TVector3f)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    if (nullQ(&theRotation))
        return;
    
    [self makeUndoSnapshotOfEntities:theEntities];
    
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
        [entity rotate:&theRotation center:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)flipEntities:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter {
    NSAssert(theEntities != nil, @"entity set must not be nil");
    
    if ([theEntities count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] flipEntities:[[theEntities copy] autorelease] axis:theAxis center:theCenter];
    
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
        [entity flipAxis:theAxis center:&theCenter];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertiesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
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

- (void)moveBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theBrushes forKey:BrushesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }

    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    NSMutableDictionary* entityBrushes = [[NSMutableDictionary alloc] init];
    NSMutableArray* changedEntities = [[NSMutableArray alloc] init];
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject])) {
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
    NSEnumerator* changedEntityEn = [changedEntities objectEnumerator];
    id <Entity> changedEntity;
    while ((changedEntity = [changedEntityEn nextObject])) {
        NSArray* brushArray = [entityBrushes objectForKey:[changedEntity entityId]];
        [[undoManager prepareWithInvocationTarget:self] moveBrushesToEntity:changedEntity brushes:brushArray];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theTemplate != nil, @"brush template must not be nil");
    
    if (!boundsContainBounds(&worldBounds, [theTemplate bounds])) {
        NSLog(@"brush template is not within world bounds");
        return nil;
    }
    
    id <Brush> brush = [[MutableBrush alloc] initWithWorldBounds:&worldBounds brushTemplate:theTemplate];
    
    NSArray* brushArray = [[NSArray alloc] initWithObjects:brush, nil];
    [self addBrushesToEntity:theEntity brushes:brushArray];
    [brushArray release];
    
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
    
    NSArray* brushArray = [[NSArray alloc] initWithObjects:brush, nil];
    [self addBrushesToEntity:theEntity brushes:brushArray];
    [brushArray release];
    
    return [brush autorelease];
}

- (void)duplicateBrushes:(NSArray *)theBrushes newBrushes:(NSMutableArray *)theNewBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSAssert(theNewBrushes != nil, @"new brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> newBrush = [self createBrushInEntity:[self worldspawn:YES] fromTemplate:brush];
        [theNewBrushes addObject:newBrush];
    }
}

- (void)translateBrushes:(NSArray *)theBrushes delta:(TVector3i)theDelta lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    TVector3i inverse;
    scaleV3i(&theDelta, -1, &inverse);
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBrushes:[[theBrushes copy] autorelease] delta:inverse lockTextures:lockTextures];
    
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
        [brush translateBy:&theDelta lockTextures:lockTextures];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateBrushes90CW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushes90CCW:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
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
        [brush rotate90CW:theAxis center:&theCenter lockTextures:lockTextures];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateBrushes90CCW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] rotateBrushes90CW:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
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
        [brush rotate90CCW:theAxis center:&theCenter lockTextures:lockTextures];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)rotateBrushes:(NSArray *)theBrushes rotation:(TQuaternion)theRotation center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;

    if (nullQ(&theRotation))
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
    
    [self makeUndoSnapshotOfBrushes:[[theBrushes copy] autorelease]];

    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject]))
        [brush rotate:&theRotation center:&theCenter lockTextures:lockTextures];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [undoManager endUndoGrouping];
}

- (void)flipBrushes:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] flipBrushes:[[theBrushes copy] autorelease] axis:theAxis center:theCenter lockTextures:lockTextures];
    
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
        [brush flipAxis:theAxis center:&theCenter lockTextures:lockTextures];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)snapBrushes:(NSArray *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    [self makeUndoSnapshotOfBrushes:theBrushes];

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
        [brush snap];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)deleteBrushes:(NSArray *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableArray* affectedEntities = [[NSMutableArray alloc] init];
    NSMutableDictionary* entityIdToBrushSet = [[NSMutableDictionary alloc] init];
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    MutableBrush* brush;
    while ((brush = [brushEn nextObject])) {
        id <Entity> entity = [brush entity];
        NSMutableArray* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        if (entityBrushes == nil) {
            entityBrushes = [[NSMutableArray alloc] init];
            [entityIdToBrushSet setObject:entityBrushes forKey:[entity entityId]];
            [entityBrushes release];
            
            [affectedEntities addObject:entity];
        }
        
        [entityBrushes addObject:brush];
    }
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    NSMutableArray* emptyEntities = [[NSMutableArray alloc] init];
    
    NSEnumerator* entityEn = [affectedEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        NSArray* entityBrushes = [entityIdToBrushSet objectForKey:[entity entityId]];
        [selectionManager removeBrushes:entityBrushes record:YES];
        [[undoManager prepareWithInvocationTarget:self] addBrushesToEntity:entity brushes:[[entityBrushes copy] autorelease]];

        NSMutableDictionary* userInfo;
        if ([self postNotifications]) {
            userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:entityBrushes forKey:BrushesKey];
            [center postNotificationName:BrushesWillBeRemoved object:self userInfo:userInfo];
        }
        
        NSEnumerator* brushEn = [entityBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject]))
            [entity removeBrush:brush];
        
        if ([self postNotifications]) {
            [center postNotificationName:BrushesWereRemoved object:self userInfo:userInfo];
            [userInfo release];
        }

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

- (void)setFaces:(NSArray *)theFaces xOffset:(int)theXOffset {
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

    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)setFaces:(NSArray *)theFaces yOffset:(int)theYOffset {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)translateFaceOffsets:(NSArray *)theFaces xDelta:(int)theXDelta yDelta:(int)theYDelta {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return;

    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateFaceOffsets:[[theFaces copy] autorelease] xDelta:-theXDelta yDelta:-theYDelta];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theFaces forKey:FacesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FacesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)setFaces:(NSArray *)theFaces xScale:(float)theXScale {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)setFaces:(NSArray *)theFaces yScale:(float)theYScale {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)scaleFaces:(NSArray *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)setFaces:(NSArray *)theFaces rotation:(float)theAngle {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)rotateFaces:(NSArray *)theFaces angle:(float)theAngle {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (void)setFaces:(NSArray *)theFaces texture:(Texture *)theTexture {
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
    
    [self makeUndoSnapshotOfFaces:[[theFaces copy] autorelease]];
    
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

- (BOOL)dragFaces:(NSArray *)theFaces distance:(float)theDistance lockTextures:(BOOL)lockTextures {
    NSAssert(theFaces != nil, @"face set must not be nil");
    
    if ([theFaces count] == 0)
        return NO;
    
    if (theDistance == 0)
        return NO;
    
    BOOL canDrag = YES;
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        NSMutableArray* brushes = [[NSMutableArray alloc] init];
        NSEnumerator* faceEn = [theFaces objectEnumerator];
        MutableFace* face;
        while ((face = [faceEn nextObject]) && canDrag) {
            MutableBrush* brush = [face brush];
            [brushes addObject:brush];
            canDrag &= [brush canDrag:face by:theDistance];
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
        while ((face = [faceEn nextObject]) && canDrag) {
            MutableBrush* brush = [face brush];
            canDrag &= [brush canDrag:face by:theDistance];
        }
    }

    if (!canDrag)
        return NO;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] dragFaces:[[theFaces copy] autorelease] distance:-theDistance lockTextures:lockTextures];

    NSEnumerator* faceEn = [theFaces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject])) {
        MutableBrush* brush = [face brush];
        [brush drag:face by:theDistance lockTexture:lockTextures];
    }
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
    
    return YES;
}

- (int)dragVertex:(int)theVertexIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    if (nullV3f(theDelta))
        return NO;
    
    MutableBrush* mutableBrush = (MutableBrush *)theBrush;
    NSArray* brushArray = [[NSArray alloc] initWithObjects:theBrush, nil];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
            userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:brushArray forKey:BrushesKey];
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:BrushesWillChange object:self userInfo:userInfo];
    }
    
    [self makeUndoSnapshotOfBrushes:brushArray];
    int newIndex = [mutableBrush dragVertex:theVertexIndex by:theDelta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushesDidChange object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [brushArray release];
    return newIndex;
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
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntities forKey:EntitiesKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntitiesWillBeRemoved object:self userInfo:userInfo];
    }
    
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        [entity setMap:nil];
        [entities removeObject:entity];
        if (worldspawn == entity)
            worldspawn = nil;
    }

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntitiesWereRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
}

- (void)removeEntity:(MutableEntity *)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    NSArray* entityArray = [[NSArray alloc] initWithObjects:theEntity, nil];
    [self removeEntities:entityArray];
    [entityArray release];
}

- (id <Entity>)worldspawn:(BOOL)create {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        NSEnumerator* en = [entities objectEnumerator];
        while ((worldspawn = [en nextObject]))
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

        if ([self postNotifications]) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:[NSArray arrayWithObject:entity] forKey:EntitiesKey];
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:EntitiesAdded object:self userInfo:userInfo];
            [userInfo release];
        }        
        
        worldspawn = entity;
        [entity release];
    }
    
    return worldspawn;
}

- (NSArray *)entities {
    return entities;
}

@end
