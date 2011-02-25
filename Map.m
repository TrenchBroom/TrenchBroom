//
//  Map.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Map.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

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

@implementation Map

- (id)init {
    if (self = [super init]) {
        entities = [[NSMutableArray alloc] init];
        worldspawn = nil;
        worldSize = 8192;
        postNotifications = YES;
    }
    
    return self;
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
    [[undoManager prepareWithInvocationTarget:self] removeEntity:theEntity];
    
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
    [[undoManager prepareWithInvocationTarget:self] addEntity:entity];
    
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

- (int)worldSize {
    return worldSize;
}

- (BOOL)postNotifications {
    return postNotifications;
}

- (NSSet *)textureNames {
    NSMutableSet* textureNames = [[NSMutableSet alloc] init];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    Entity* entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            Face* face;
            while ((face = [faceEn nextObject]))
                [textureNames addObject:[face texture]];
        }
    }
    
    return [textureNames autorelease];
}

- (void)setPostNotifications:(BOOL)value {
    postNotifications = value;
}

- (NSUndoManager *)undoManager {
    return undoManager;
}

- (void)setUndoManager:(NSUndoManager *)theUndoManager {
    [undoManager release];
    undoManager = [theUndoManager retain];
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
}

- (void)faceRemoved:(Face *)face {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FaceRemoved object:self userInfo:[NSDictionary dictionaryWithObject:face forKey:FaceKey]];
        [center postNotificationName:BrushChanged object:self userInfo:[NSDictionary dictionaryWithObject:[face brush] forKey:BrushKey]];
    }
}

- (void)brushAdded:(Brush *)brush {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }
}

- (void)brushRemoved:(Brush *)brush {
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:BrushRemoved object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:BrushKey]];
    }
}

- (void)propertyAdded:(Entity *)entity key:(NSString *)key value:(NSString *)value {
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
    [undoManager release];
    [entities release];
    [super dealloc];
}

@end
