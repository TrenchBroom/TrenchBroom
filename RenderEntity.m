//
//  RenderEntity.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderEntity.h"
#import "RenderMap.h"
#import "BrushFigure.h"
#import "Entity.h"
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
    BrushFigure* renderBrush = [[BrushFigure alloc] initInEntity:self withBrush:theBrush faceVBO:faceVBO];
    [renderBrushes setObject:renderBrush forKey:[theBrush brushId]];
    [renderBrush release];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];
    [self createRenderBrushFor:brush];
    [renderMap entityChanged];
}

- (void)brushRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];
    [renderBrushes removeObjectForKey:[brush brushId]];
    [renderMap entityChanged];
}

- (id)initInMap:(RenderMap *)theRenderMap withEntity:(Entity *)theEntity faceVBO:(VBOBuffer *)theFaceVBO {
    if (theRenderMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"render map must not be nil"];
    if (theEntity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];

    if (self = [self init]) {
        renderMap = [theRenderMap retain];
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

- (void)brushChanged {
    [renderMap entityChanged];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceVBO release];
    [renderBrushes release];
    [entity release];
    [renderMap release];
    [super dealloc];
}

@end
