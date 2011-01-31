//
//  RenderMap.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderMap.h"
#import "Map.h"
#import "Entity.h"
#import "RenderEntity.h"
#import "VBOBuffer.h"

@implementation RenderMap

- (id)init {
    if (self = [super init]) {
        map = nil;
        renderEntities = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithMap:(Map *)theMap faceVBO:(VBOBuffer *)theFaceVBO edgeVBO:(VBOBuffer *)theEdgeVBO {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];
    if (theEdgeVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge VBO buffer must not be nil"];

    if (self = [self init]) {
        map = [theMap retain];
        faceVBO = [theFaceVBO retain];
        edgeVBO = [theEdgeVBO retain];
        
        NSArray* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        
        while ((entity = [entityEn nextObject])) {
            RenderEntity* renderEntity = [[RenderEntity alloc] initWithEntity:entity faceVBO:faceVBO edgeVBO:edgeVBO];
            [renderEntities setObject:renderEntity forKey:[entity entityId]];
            [renderEntity release];
        }
    }
    
    return self;
}

- (NSArray *)renderEntities {
    return [renderEntities allValues];
}

- (void)dealloc {
    [faceVBO release];
    [edgeVBO release];
    [renderEntities release];
    [map release];
    [super dealloc];
}

@end
