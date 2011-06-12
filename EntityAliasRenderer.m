//
//  AliasRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityAliasRenderer.h"
#import "Entity.h"
#import "EntityDefinition.h"
#import "EntityDefinitionProperty.h"
#import "ModelProperty.h"
#import "AliasManager.h"
#import "Alias.h"
#import "AliasRenderer.h"
#import "VBOBuffer.h"
#import "Filter.h"

@implementation EntityAliasRenderer

- (id)init {
    if ((self = [super init])) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        entities = [[NSMutableSet alloc] init];
        aliasRenderers = [[NSMutableDictionary alloc] init];
        entityRenderers = [[NSMutableDictionary alloc] init];

        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        palette = [[NSData alloc] initWithContentsOfFile:palettePath];
    }
    
    return self;
}

- (void)dealloc {
    [entities release];
    [aliasRenderers release];
    [entityRenderers release];
    [vbo release];
    [palette release];
    [filter release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must no be nil");
    
    EntityDefinition* definition = [entity entityDefinition];
    NSArray* properties = [definition properties];

    NSEnumerator* propertyEn = [properties objectEnumerator];
    id <EntityDefinitionProperty> property;
    while ((property = [propertyEn nextObject]))
        if ([property type] == EDP_MODEL)
            break;
    
    if (property != nil) {
        ModelProperty* modelProperty = (ModelProperty *)property;
        NSString* modelName = [[modelProperty modelPath] substringFromIndex:1];
        AliasRenderer* aliasRenderer = [aliasRenderers objectForKey:modelName];
        if (aliasRenderer == nil) {
            NSArray* pakPaths = [NSArray arrayWithObject:@"/Applications/Quake/id1"];

            AliasManager* aliasManager = [AliasManager sharedManager];
            Alias* alias = [aliasManager aliasWithName:modelName paths:pakPaths];
            
            if (alias != nil) {
                aliasRenderer = [[AliasRenderer alloc] initWithAlias:alias vbo:vbo palette:palette];
                [aliasRenderers setObject:aliasRenderer forKey:modelName];
                [aliasRenderer release];
            }
        }
        
        if (aliasRenderer != nil) {
            [entities addObject:entity];
            [entityRenderers setObject:aliasRenderer forKey:[entity entityId]];
        }
    }
}

- (void)removeEntity:(id <Entity>)entity {
    [entities removeObject:entity];
    [entityRenderers removeObjectForKey:[entity entityId]];
}

- (void)render {
    [vbo activate];
    
    glMatrixMode(GL_MODELVIEW);
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if (filter == nil || [filter isEntityRenderable:entity]) {
            glPushMatrix();
            AliasRenderer* aliasRenderer = [entityRenderers objectForKey:[entity entityId]];
            [aliasRenderer renderWithEntity:entity];
            glPopMatrix();
        }
    }
    
    [vbo deactivate];
}

- (void)setFilter:(id <Filter>)theFilter {
    [filter release];
    filter = [theFilter retain];
}

@end
