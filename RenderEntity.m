//
//  RenderEntity.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderEntity.h"
#import "Entity.h"
#import "RenderBrush.h"
#import "Brush.h"
#import "VBOBuffer.h"

@implementation RenderEntity

- (id)init {
    if (self = [super init]) {
        renderBrushes = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)createRenderBrushFor:(Brush *)theBrush {
    RenderBrush* renderBrush = [[RenderBrush alloc] initWithBrush:theBrush faceVBO:faceVBO];
    [renderBrushes setObject:renderBrush forKey:[theBrush brushId]];
    [renderBrush release];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];
    [self createRenderBrushFor:brush];
}

- (void)brushRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];
    [renderBrushes removeObjectForKey:[brush brushId]];
}

- (id)initWithEntity:(Entity *)theEntity faceVBO:(VBOBuffer *)theFaceVBO {
    if (theEntity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];

    if (self = [self init]) {
        entity = [theEntity retain];
        faceVBO = [theFaceVBO retain];
        
        NSArray* brushes = [entity brushes];
        NSEnumerator* brushEn = [brushes objectEnumerator];
        Brush* brush;
        
        while ((brush = [brushEn nextObject]))
            [self createRenderBrushFor:brush];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(brushAdded:) name:EntityBrushAdded object:entity];
        [center addObserver:self selector:@selector(brushRemoved:) name:EntityBrushRemoved object:entity];
    }
    
    return self;
}

- (NSArray *)renderBrushes {
    return [renderBrushes allValues];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceVBO release];
    [renderBrushes release];
    [entity release];
    [super dealloc];
}

@end
