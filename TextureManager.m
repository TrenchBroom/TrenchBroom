//
//  TextureManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureManager.h"
#import <OpenGL/gl.h>
#import "Texture.h"
#import "TextureCollection.h"

NSString* const TextureManagerChanged = @"TextureManagerChanged";

NSString* const UnknownTextureNameException = @"UnknownTextureNameException";
NSString* const MissingPaletteException = @"MissingPaletteException";

@interface TextureManager (private)

- (void)validate;

@end

@implementation TextureManager (private)

- (void)validate {
    if (!valid) {
        [textures removeAllObjects];
        [texturesByName removeAllObjects];
        
        NSEnumerator* collectionEn = [textureCollections objectEnumerator];
        TextureCollection* collection;
        while ((collection = [collectionEn nextObject])) {
            NSEnumerator* textureEn = [[collection textures] objectEnumerator];
            Texture* texture;
            while ((texture = [textureEn nextObject]))
                [textures setObject:texture forKey:[texture name]];
        }
        
        [texturesByName addObjectsFromArray:[textures allValues]];
        [texturesByName sortUsingSelector:@selector(compareByName:)];
        
        valid = YES;
    }
}

@end

@implementation TextureManager

- (id)init {
    if (self = [super init]) {
        textureCollections = [[NSMutableArray alloc] init];
        textures = [[NSMutableDictionary alloc] init];
        texturesByName = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)addTextureCollection:(TextureCollection *)theCollection {
    NSAssert(theCollection != nil, @"texture collection must not be nil");
    [textureCollections addObject:theCollection];
    valid = NO;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:TextureManagerChanged object:self];
}

- (void)removeTextureCollection:(NSString *)theName {
    NSAssert(theName != nil, @"name must not be nil");
    
    int index;
    for (index = 0; index < [textureCollections count]; index++) {
        TextureCollection* collection = [textureCollections objectAtIndex:index];
        if ([[collection name] isEqualToString:theName])
            break;
    }
    
    if (index < [textureCollections count]) {
        [textureCollections removeObjectAtIndex:index];
        valid = NO;

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:TextureManagerChanged object:self];
    }
}

- (void)removeAllTextureCollections {
    if ([textureCollections count] == 0)
        return;
    
    [textureCollections removeAllObjects];
    valid = NO;

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:TextureManagerChanged object:self];
}

- (NSArray *)textureCollections {
    return textureCollections;
}

- (Texture *)textureForName:(NSString *)name {
    NSAssert(name != nil, @"name must not be nil");
    
    [self validate];
    return [textures objectForKey:name];
}

- (NSArray *)texturesByName {
    [self validate];
    return texturesByName;
}

- (void)activateTexture:(NSString *)name {
    Texture* texture = [self textureForName:name];
    if (texture == nil)
        [NSException raise:UnknownTextureNameException format:@"unknown texture name: %@", name];

    [texture activate];
}

- (void)deactivateTexture {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (NSString *)wadProperty {
    NSMutableString* wadProperty = [[NSMutableString alloc] init];
    
    for (int i = 0; i < [textureCollections count]; i++) {
        TextureCollection* collection = [textureCollections objectAtIndex:i];
        [wadProperty appendString:[collection name]];
        if (i < [textureCollections count] - 1)
            [wadProperty appendString:@";"];
    }
    
    return [wadProperty autorelease];
}

- (void)dealloc {
    [texturesByName release];
    [textures release];
    [textureCollections release];
    [super dealloc];
}

@end
