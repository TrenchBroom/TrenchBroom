//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "MutableFace.h"
#import "TextureManager.h"
#import "Picker.h"
#import "GLResources.h"
#import "WadLoader.h"
#import "MapWindowController.h"
#import "ProgressWindowController.h"
#import "MapParser.h"
#import "Vector3i.h"

NSString* const FaceAdded           = @"FaceAdded";
NSString* const FaceRemoved         = @"FaceRemoved";
NSString* const FaceFlagsChanged    = @"FaceFlagsChanged";
NSString* const FaceTextureChanged  = @"FaceTextureChanged";
NSString* const FaceGeometryChanged = @"FaceGeometryChanged";
NSString* const FaceKey             = @"Face";
NSString* const FaceOldTextureKey   = @"FaceOldTexture";
NSString* const FaceNewTextureKey   = @"FaceNewTexture";

NSString* const BrushAdded          = @"BrushAdded";
NSString* const BrushRemoved        = @"BrushRemoved";
NSString* const BrushChanged        = @"BrushChanged";
NSString* const BrushKey            = @"Brush";
NSString* const BrushOldBoundsKey   = @"BrushOldBounds";
NSString* const BrushNewBoundsKey   = @"BrushNewBounds";

NSString* const EntityAdded         = @"EntityAdded";
NSString* const EntityRemoved       = @"EntityRemoved";
NSString* const EntityKey           = @"Entity";

NSString* const PropertyAdded       = @"PropertyAdded";
NSString* const PropertyRemoved     = @"PropertyRemoved";
NSString* const PropertyChanged     = @"PropertyChanged";
NSString* const PropertyKeyKey      = @"PropertyKey";
NSString* const PropertyOldValueKey = @"PropertyOldValue";
NSString* const PropertyNewValueKey = @"PropertyNewValue";

@implementation MapDocument

- (id)init {
    if (self = [super init]) {
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
    TextureManager* textureManager = [glResources textureManager];
    [textureManager removeAllTextures];
    
    NSString* wads = [[self worldspawn] propertyForKey:@"wad"];
    if (wads != nil) {
        [[glResources openGLContext] makeCurrentContext];
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
                
                [textureManager loadTexturesFrom:wad];
            }
        }
    }
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject]))
                [textureManager incUsageCount:[face texture]];
        }
    }
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
    
    [self setPostNotifications:YES];
    [[self undoManager] enableUndoRegistration];
    
    [pwc close];
    [pwc release];
    
    [picker release];
    picker = [[Picker alloc] initWithDocument:self];
    [self refreshWadFiles];
    
    return YES;
}

- (id <Entity>)worldspawn {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        NSEnumerator* en = [entities objectEnumerator];
        while ((worldspawn = [en nextObject]))
            if ([worldspawn isWorldspawn])
                break;
    }
    
    return worldspawn;
}

- (id <Entity>)createEntity {
    MutableEntity* entity = [[MutableEntity alloc] init];
    [self addEntity:entity];
    return [entity autorelease];
}

- (void)addEntity:(MutableEntity *)theEntity {
    [[[self undoManager] prepareWithInvocationTarget:self] removeEntity:theEntity];
    
    [entities addObject:theEntity];
    [theEntity setMap:self];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:[NSDictionary dictionaryWithObject:theEntity forKey:EntityKey]];
    }
}

- (void)removeEntity:(MutableEntity *)theEntity {
    [[[self undoManager] prepareWithInvocationTarget:self] addEntity:theEntity];
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:theEntity forKey:EntityKey];
    
    [theEntity setMap:nil];
    [entities removeObject:theEntity];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:userInfo];
    }
}

- (NSArray *)entities {
    return entities;
}

- (void)setFace:(id <Face>)face xOffset:(int)xOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xOffset:[face xOffset]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setXOffset:xOffset];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)setFace:(id <Face>)face yOffset:(int)yOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yOffset:[face yOffset]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setYOffset:yOffset];

    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)translateFaceOffset:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta {
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];

    [self setFace:face xOffset:[face xOffset] + xDelta];
    [self setFace:face yOffset:[face yOffset] + yDelta];

    [undoManager endUndoGrouping];
}

- (void)setFace:(id <Face>)face xScale:(float)xScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xScale:[face xScale]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setXScale:xScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)setFace:(id <Face>)face yScale:(float)yScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yScale:[face yScale]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setYScale:yScale];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)setFace:(id <Face>)face rotation:(float)angle {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face rotation:[face rotation]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setRotation:angle];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)setFace:(id <Face>)face texture:(NSString *)texture {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face texture:[NSString stringWithString:[face texture]]];
    
    MutableFace* mutableFace = (MutableFace *)face;
    [mutableFace setTexture:texture];
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity {
    MutableBrush* brush = [[MutableBrush alloc] init];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    [mutableEntity addBrush:brush];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrush:brush];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }
    
    return [brush autorelease];
}

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    MutableBrush* brush = [[MutableBrush alloc] initWithTemplate:theTemplate];
    
    MutableEntity* mutableEntity = (MutableEntity *)theEntity;
    [mutableEntity addBrush:brush];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrush:brush];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }

    return [brush autorelease];
}

- (void)deleteBrush:(id <Brush>)brush {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] createBrushInEntity:[brush entity] fromTemplate:brush];
    
    MutableEntity* mutableEntity = (MutableEntity *)[brush entity];
    [mutableEntity removeBrush:brush];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushRemoved object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }
}

- (void)translateBrush:(id <Brush>)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] translateBrush:brush xDelta:-xDelta yDelta:-yDelta zDelta:-zDelta];

    BoundingBox* oldBounds = [[BoundingBox alloc] initWithBounds:[brush bounds]];
    
    MutableBrush* mutableBrush = (MutableBrush *)brush;
    [mutableBrush translateBy:[Vector3i vectorWithX:xDelta y:yDelta z:zDelta]];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brush forKey:BrushKey];
        [userInfo setObject:oldBounds forKey:BrushOldBoundsKey];
        [userInfo setObject:[brush bounds] forKey:BrushNewBoundsKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushChanged object:self userInfo:userInfo];
        [userInfo release];
    }
    
    [oldBounds release];
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

- (void)dealloc {
    [entities release];
    [picker release];
    [glResources release];
    [super dealloc];
}

@end
