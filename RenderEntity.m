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

@implementation RenderEntity

- (id)init {
    if (self = [super init]) {
        renderBrushes = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

extern NSString* const EntityBrushAdded;
extern NSString* const EntityBrushRemoved;

extern NSString* const EntityBrush;

extern NSString* const EntityPropertyAdded;
extern NSString* const EntityPropertyRemoved;
extern NSString* const EntityPropertyChanged;

- (id)initWithEntity:(Entity *)anEntity {
    if (anEntity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];

    if (self = [self init]) {
        entity = [anEntity retain];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(brushAdded:) name:EntityBrushAdded object:entity];
        [center addObserver:self selector:@selector(brushRemoved:) name:EntityBrushRemoved object:entity];
        [center addObserver:self selector:@selector(propertyAdded:) name:EntityPropertyAdded object:entity];
        [center addObserver:self selector:@selector(propertyRemoved:) name:EntityPropertyRemoved object:entity];
        [center addObserver:self selector:@selector(propertyChanged:) name:EntityPropertyChanged object:entity];
        
        NSSet* brushes = [entity brushes];
        NSEnumerator* brushEn = [brushes objectEnumerator];
        Brush* brush;
        
        while ((brush = [brushEn nextObject])) {
            RenderBrush* renderBrush = [[RenderBrush alloc] initWithBrush:brush];
            [renderBrushes setObject:renderBrush forKey:[brush getId]];
            [renderBrush release];
        }
    }
    
    return self;
}

- (void)brushAdded:(NSNotification *)notification{
    Brush* brush = [notification object];
    RenderBrush* renderBrush = [[RenderBrush alloc] initWithBrush:brush];
    [renderBrushes setObject:renderBrush forKey:[brush getId]];
    [renderBrush release];
}

- (void)brushRemoved:(NSNotification *)notification {
    Brush* brush = [notification object];
    [renderBrushes removeObjectForKey:[brush getId]];
}

- (void)propertyAdded:(NSNotification *)notification {
}

- (void)propertyRemoved:(NSNotification *)notification {
}

- (void)propertyChanged:(NSNotification *)notification {
}

- (void)renderWithContext:(id <RenderContext>)renderContext {
    if (renderContext == nil)
        [NSException raise:NSInvalidArgumentException format:@"render context must not be nil"];
    
    NSEnumerator* en = [renderBrushes objectEnumerator];
    RenderBrush* brush;
    
    while ((brush = [en nextObject]))
        [brush renderWithContext:renderContext];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [renderBrushes release];
    [entity release];
    [super dealloc];
}

@end
