//
//  EntityRendererManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityRendererManager.h"
#import "EntityRenderer.h"
#import "EntityDefinition.h"
#import "Entity.h"
#import "ModelProperty.h"
#import "AliasManager.h"
#import "Alias.h"
#import "BspManager.h"
#import "Bsp.h"
#import "AliasRenderer.h"
#import "BspRenderer.h"
#import "VBOBuffer.h"

@interface EntityRendererManager (private)

- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty;
- (id <EntityRenderer>)entityRendererForModelProperty:(ModelProperty *)theModelProperty;

@end

@implementation EntityRendererManager (private)

- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty {
    return [NSString stringWithFormat:@"%@ %@ %i", [theModelProperty modelPath], [theModelProperty flagName], [theModelProperty skinIndex]];
}

- (id <EntityRenderer>)entityRendererForModelProperty:(ModelProperty *)theModelProperty {
    id <NSCopying> rendererKey = [self rendererKey:theModelProperty];
    id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:rendererKey];
    if (entityRenderer == nil) {
        NSString* modelName = [[theModelProperty modelPath] substringFromIndex:1];
        NSArray* pakPaths = [NSArray arrayWithObject:@"/Applications/Quake/id1"];
        if ([[modelName pathExtension] isEqualToString:@"mdl"]) {
            AliasManager* aliasManager = [AliasManager sharedManager];
            Alias* alias = [aliasManager aliasWithName:modelName paths:pakPaths];
            
            if (alias != nil) {
                int skinIndex = [theModelProperty skinIndex];
                
                entityRenderer = [[AliasRenderer alloc] initWithAlias:alias skinIndex:skinIndex vbo:vbo palette:palette];
                [entityRenderers setObject:entityRenderer forKey:rendererKey];
                [entityRenderer release];
            }
        } else if ([[modelName pathExtension] isEqualToString:@"bsp"]) {
            BspManager* bspManager = [BspManager sharedManager];
            Bsp* bsp = [bspManager bspWithName:modelName paths:pakPaths];
            
            if (bsp != nil) {
                entityRenderer = [[BspRenderer alloc] initWithBsp:bsp vbo:vbo palette:palette];
                [entityRenderers setObject:entityRenderer forKey:rendererKey];
                [entityRenderer release];
            }
        }
    }
    
    return entityRenderer;
}

@end

@implementation EntityRendererManager

- (id)initWithPalette:(NSData *)thePalette {
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        entityRenderers = [[NSMutableDictionary alloc] init];
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        palette = [thePalette retain];
    }
    
    return self;
}

- (void)dealloc {
    [entityRenderers release];
    [vbo release];
    [palette release];
    [super dealloc];
}

- (id <EntityRenderer>)entityRendererForDefinition:(EntityDefinition *)theDefinition {
    NSAssert(theDefinition != nil, @"entity definition must not be nil");
    
    ModelProperty* modelProperty = [theDefinition defaultModelProperty];
    if (modelProperty == nil)
        return nil;
    
    return [self entityRendererForModelProperty:modelProperty];
}

- (id <EntityRenderer>)entityRendererForEntity:(id<Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    EntityDefinition* definition = [theEntity entityDefinition];
    if (definition == nil)
        return nil;
    
    ModelProperty* modelProperty = [definition modelPropertyForEntity:theEntity];
    if (modelProperty == nil)
        return nil;
    
    return [self entityRendererForModelProperty:modelProperty];
}

- (void)activate {
    [vbo activate];
}

- (void)deactivate {
    [vbo deactivate];
}


@end
