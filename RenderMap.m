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

- (id)initWithMap:(Map *)theMap vboBuffer:(VBOBuffer *)theVboBuffer {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    if (theVboBuffer == nil)
        [NSException raise:NSInvalidArgumentException format:@"VBO buffer must not be nil"];

    if (self = [self init]) {
        map = [theMap retain];
        vboBuffer = [theVboBuffer retain];
        
        NSArray* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        
        while ((entity = [entityEn nextObject])) {
            RenderEntity* renderEntity = [[RenderEntity alloc] initWithEntity:entity vboBuffer:vboBuffer];
            [renderEntities setObject:renderEntity forKey:[entity getId]];
            [renderEntity release];
        }
    }
    
    return self;
}

- (void)renderWithContext:(id <RenderContext>)renderContext {
    if (renderContext == nil)
        [NSException raise:NSInvalidArgumentException format:@"render context must not be nil"];
    
    NSEnumerator* en = [renderEntities objectEnumerator];
    RenderEntity* entity;
    
    while ((entity = [en nextObject]))
        [entity renderWithContext:renderContext];
}

- (void)dealloc {
    [vboBuffer release];
    [renderEntities release];
    [map release];
    [super dealloc];
}

@end
