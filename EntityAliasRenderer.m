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
#import "BspManager.h"
#import "Bsp.h"
#import "BspModel.h"
#import "EntityRenderer.h"
#import "BspRenderer.h"
#import "AliasRenderer.h"
#import "VBOBuffer.h"
#import "Filter.h"

@implementation EntityAliasRenderer

- (id)init {
    if ((self = [super init])) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        entities = [[NSMutableSet alloc] init];
        entityRenderers = [[NSMutableDictionary alloc] init];

        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        palette = [[NSData alloc] initWithContentsOfFile:palettePath];
    }
    
    return self;
}

- (void)dealloc {
    [entities release];
    [entityRenderers release];
    [vbo release];
    [palette release];
    [filter release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must no be nil");
    
    EntityDefinition* definition = [entity entityDefinition];
    if (definition == nil)
        return;
    
    NSString* definitionName = [definition name];
    id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:definitionName];
    if (entityRenderer == nil) {
        NSArray* properties = [definition properties];
        NSEnumerator* propertyEn = [properties objectEnumerator];
        id <EntityDefinitionProperty> property;
        while ((property = [propertyEn nextObject]))
            if ([property type] == EDP_MODEL)
                break;
        
        // TODO factor out model loading and load models for entity definitions that do not have a model key (e.g. health)
        
        // see http://inside3d.com/browse.php?show=items.qc
        if (property != nil) {
            ModelProperty* modelProperty = (ModelProperty *)property;
            NSString* modelName = [[modelProperty modelPath] substringFromIndex:1];
            entityRenderer = [entityRenderers objectForKey:modelName];
            if (entityRenderer == nil) {
                NSArray* pakPaths = [NSArray arrayWithObject:@"/Applications/Quake/id1"];
                if ([[modelName pathExtension] isEqualToString:@"mdl"]) {
                    AliasManager* aliasManager = [AliasManager sharedManager];
                    Alias* alias = [aliasManager aliasWithName:modelName paths:pakPaths];
                    
                    if (alias != nil) {
                        entityRenderer = [[AliasRenderer alloc] initWithAlias:alias vbo:vbo palette:palette];
                        [entityRenderers setObject:entityRenderer forKey:definitionName];
                        [entityRenderer release];
                    }
                } else {
                    BspManager* bspManager = [BspManager sharedManager];
                    Bsp* bsp = [bspManager bspWithName:modelName paths:pakPaths];
                    
                    if (bsp != nil) {
                        entityRenderer = [[BspRenderer alloc] initWithBsp:bsp vbo:vbo];
                        [entityRenderers setObject:entityRenderer forKey:definitionName];
                        [entityRenderer release];
                    }
                }
            }
        }
    }

    if (entityRenderer != nil)
        [entities addObject:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    [entities removeObject:entity];
}

- (void)render {
    [vbo activate];
    
    glMatrixMode(GL_MODELVIEW);
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if (filter == nil || [filter isEntityRenderable:entity]) {
            glPushMatrix();
            EntityDefinition* definition = [entity entityDefinition];
            NSString* definitionName = [definition name];
            id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:definitionName];
            [entityRenderer renderWithEntity:entity];
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
