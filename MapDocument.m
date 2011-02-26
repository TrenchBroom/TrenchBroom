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
    Entity* entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            Face* face;
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

- (Entity *)worldspawn {
    if (worldspawn == nil || ![worldspawn isWorldspawn]) {
        NSEnumerator* en = [entities objectEnumerator];
        while ((worldspawn = [en nextObject]))
            if ([worldspawn isWorldspawn])
                break;
    }
    
    return worldspawn;
}

- (void)addEntity:(Entity *)theEntity {
    [[[self undoManager] prepareWithInvocationTarget:self] removeEntity:theEntity];
    
    [entities addObject:theEntity];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:[NSDictionary dictionaryWithObject:theEntity forKey:EntityKey]];
    }
}

- (Entity *)createEntity {
    Entity* entity = [[Entity alloc] initInMap:self];
    [self addEntity:entity];
    return [entity autorelease];
}

- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value {
    Entity* entity = [[Entity alloc] initInMap:self property:key value:value];
    [self addEntity:entity];
    return entity;
}

- (void)removeEntity:(Entity *)entity {
    [[[self undoManager] prepareWithInvocationTarget:self] addEntity:entity];
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:entity forKey:EntityKey];
    [entities removeObject:entity];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityAdded object:self userInfo:userInfo];
    }
}

- (NSArray *)entities {
    return entities;
}

- (void)setFace:(Face *)face xOffset:(int)xOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xOffset:[face xOffset]];
    
    [face setXOffset:xOffset];
}

- (void)setFace:(Face *)face yOffset:(int)yOffset {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yOffset:[face yOffset]];
    
    [face setYOffset:yOffset];
}

- (void)translateFaceOffset:(Face *)face xDelta:(int)xDelta yDelta:(int)yDelta {
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];

    [self setFace:face xOffset:[face xOffset] + xDelta];
    [self setFace:face yOffset:[face yOffset] + yDelta];

    [undoManager endUndoGrouping];
}

- (void)setFace:(Face *)face xScale:(float)xScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face xScale:[face xScale]];
    
    [face setXScale:xScale];
}

- (void)setFace:(Face *)face yScale:(float)yScale {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face yScale:[face yScale]];
    
    [face setYScale:yScale];
}

- (void)setFace:(Face *)face rotation:(float)angle {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face rotation:[face rotation]];
    
    [face setRotation:angle];
}

- (void)setFace:(Face *)face texture:(NSString *)texture {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face texture:[face texture]]; // this could go wrong
    
    [face setTexture:texture];
}

- (void)setFace:(Face *)face point1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 {
    Vector3i* oldPoint1 = [[Vector3i alloc] initWithVector:[face point1]];
    Vector3i* oldPoint2 = [[Vector3i alloc] initWithVector:[face point2]];
    Vector3i* oldPoint3 = [[Vector3i alloc] initWithVector:[face point3]];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setFace:face point1:oldPoint1 point2:oldPoint2 point3:oldPoint3];
    
    [oldPoint1 release];
    [oldPoint2 release];
    [oldPoint3 release];
    
    [face setPoint1:point1 point2:point2 point3:point3];
}

- (void)translateFace:(Face *)face xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    Vector3i* newPoint1 = [[Vector3i alloc] initWithVector:[face point1]];
    [newPoint1 addX:xDelta Y:yDelta Z:zDelta];

    Vector3i* newPoint2 = [[Vector3i alloc] initWithVector:[face point2]];
    [newPoint2 addX:xDelta Y:yDelta Z:zDelta];

    Vector3i* newPoint3 = [[Vector3i alloc] initWithVector:[face point3]];
    [newPoint3 addX:xDelta Y:yDelta Z:zDelta];
    
    [self setFace:face point1:newPoint1 point2:newPoint2 point3:newPoint3];
}

- (Brush *)createBrushInEntity:(Entity *)entity fromTemplate:(Brush *)theTemplate {
    Brush* brush = [entity createBrushFromTemplate:theTemplate];
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] deleteBrush:brush];
    
    return brush;
}

- (void)deleteBrush:(Brush *)brush {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] createBrushInEntity:[brush entity] fromTemplate:brush];
    
    [[brush entity] removeBrush:brush];
}

- (void)translateBrush:(Brush *)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta {
    NSUndoManager* undoManager = [self undoManager];
    [undoManager beginUndoGrouping];
    
    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [self translateFace:face xDelta:xDelta yDelta:yDelta zDelta:zDelta];
    
    [undoManager endUndoGrouping];
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

- (void)faceFlagsChanged:(Face *)face {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceFlagsChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
    }
}

- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture {
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:face forKey:FaceKey];
        [userInfo setObject:oldTexture forKey:FaceOldTextureKey];
        [userInfo setObject:newTexture forKey:FaceNewTextureKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceTextureChanged object:self userInfo:userInfo];
    }
    
    TextureManager* textureManager = [glResources textureManager];
    [textureManager decUsageCount:oldTexture];
    [textureManager incUsageCount:newTexture];
}

- (void)faceGeometryChanged:(Face *)face {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceGeometryChanged object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
        [center postNotificationName:BrushChanged object:self userInfo:[NSDictionary dictionaryWithObject:[face brush] forKey:BrushKey]];
    }
}

- (void)faceAdded:(Face *)face {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceAdded object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
        [center postNotificationName:BrushChanged object:self userInfo:[NSDictionary dictionaryWithObject:[face brush] forKey:BrushKey]];
    }
    
    TextureManager* textureManager = [glResources textureManager];
    [textureManager incUsageCount:[face texture]];
}

- (void)faceRemoved:(Face *)face {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceRemoved object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
        [center postNotificationName:BrushChanged object:self userInfo:[NSDictionary dictionaryWithObject:[face brush] forKey:BrushKey]];
    }

    TextureManager* textureManager = [glResources textureManager];
    [textureManager decUsageCount:[face texture]];
}

- (void)brushAdded:(Brush *)brush {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }

    NSEnumerator* faces = [[brush faces] objectEnumerator];
    Face* face;

    TextureManager* textureManager = [glResources textureManager];
    while ((face = [faces nextObject]))
        [textureManager incUsageCount:[face texture]];
        
}

- (void)brushRemoved:(Brush *)brush {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushRemoved object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }
    
    NSEnumerator* faces = [[brush faces] objectEnumerator];
    Face* face;
    
    TextureManager* textureManager = [glResources textureManager];
    while ((face = [faces nextObject]))
        [textureManager decUsageCount:[face texture]];
}

- (void)propertyAdded:(Entity *)entity key:(NSString *)key value:(NSString *)value {
    if ([key isEqualToString:@"wad"])
        [self refreshWadFiles];
    
    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:entity forKey:EntityKey];
        [userInfo setObject:key forKey:PropertyKeyKey];
        [userInfo setObject:value forKey:PropertyNewValueKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertyAdded object:self userInfo:userInfo];
    }
}

- (void)propertyRemoved:(Entity *)entity key:(NSString *)key value:(NSString *)value {
    if ([key isEqualToString:@"wad"])
        [self refreshWadFiles];

    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:entity forKey:EntityKey];
        [userInfo setObject:key forKey:PropertyKeyKey];
        [userInfo setObject:value forKey:PropertyOldValueKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertyRemoved object:self userInfo:userInfo];
    }
}

- (void)propertyChanged:(Entity *)entity key:(NSString *)key oldValue:(NSString *)oldValue newValue:(NSString *)newValue {
    if ([key isEqualToString:@"wad"])
        [self refreshWadFiles];

    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:entity forKey:EntityKey];
        [userInfo setObject:key forKey:PropertyKeyKey];
        [userInfo setObject:oldValue forKey:PropertyOldValueKey];
        [userInfo setObject:newValue forKey:PropertyNewValueKey];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:PropertyChanged object:self userInfo:userInfo];
    }
}

- (void)dealloc {
    [entities release];
    [picker release];
    [glResources release];
    [super dealloc];
}

@end
