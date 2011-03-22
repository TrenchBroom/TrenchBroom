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
#import "MathCache.h"

NSString* const FaceWillChange      = @"FaceWillChange";
NSString* const FaceDidChange       = @"FaceDidChange";
NSString* const FaceKey             = @"Face";

NSString* const BrushAdded          = @"BrushAdded";
NSString* const BrushRemoved        = @"BrushRemoved";
NSString* const BrushWillChange     = @"BrushWillChange";
NSString* const BrushDidChange      = @"BrushDidChange";
NSString* const BrushKey            = @"Brush";

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

- (id <Entity>)createEntityWithProperties:(NSDictionary *)properties {
    MutableEntity* entity = [[MutableEntity alloc] initWithProperties:properties];
    [self addEntity:entity];
    return [entity autorelease];
}

- (void)addEntity:(MutableEntity *)theEntity {
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
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:theEntity forKey:EntityKey];
    }
    
    [theEntity setMap:nil];
    [entities removeObject:theEntity];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:userInfo];
        [userInfo release];
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
    [undoManager beginUndoGrouping];

    [self setFace:face xOffset:[face xOffset] + xDelta];
    [self setFace:face yOffset:[face yOffset] + yDelta];

    [undoManager endUndoGrouping];
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

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate {
    MutableBrush* brush = [[MutableBrush alloc] initWithTemplate:theTemplate];
    
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

- (void)deleteBrush:(id <Brush>)brush {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] createBrushInEntity:[brush entity] fromTemplate:brush];
    
    NSMutableDictionary* userInfo;
    if ([self postNotifications]) {
        userInfo = [[NSMutableDictionary alloc] init];
        [userInfo setObject:brush forKey:BrushKey];
    }
    
    MutableEntity* mutableEntity = (MutableEntity *)[brush entity];
    [mutableEntity removeBrush:brush];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushRemoved object:self userInfo:userInfo];
        [userInfo release];
    }
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
    
    MathCache* cache = [MathCache sharedCache];
    Vector3i* delta = [cache vector3i];
    [delta setX:xDelta];
    [delta setY:yDelta];
    [delta setZ:zDelta];
    
    MutableBrush* mutableBrush = (MutableBrush *)brush;
    [mutableBrush translateBy:delta];
    
    [cache returnVector3i:delta];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushDidChange object:self userInfo:userInfo];
        [userInfo release];
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

- (void)dealloc {
    [entities release];
    [picker release];
    [glResources release];
    [super dealloc];
}

@end
