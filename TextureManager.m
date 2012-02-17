/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        
        for (TextureCollection* collection in textureCollections)
            for (Texture* texture in [collection textures])
                [textures setObject:texture forKey:[texture name]];
        
        [texturesByName addObjectsFromArray:[textures allValues]];
        [texturesByName sortUsingSelector:@selector(compareByName:)];
        
        valid = YES;
    }
}

@end

@implementation TextureManager

- (id)init {
    if ((self = [super init])) {
        textureCollections = [[NSMutableArray alloc] init];
        textures = [[NSMutableDictionary alloc] init];
        texturesByName = [[NSMutableArray alloc] init];
        dummies = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addTextureCollection:(TextureCollection *)theCollection atIndex:(NSUInteger)theIndex {
    NSAssert(theCollection != nil, @"texture collection must not be nil");
    [textureCollections insertObject:theCollection atIndex:theIndex];
    valid = NO;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:TextureManagerChanged object:self];
}

- (void)removeTextureCollectionAtIndex:(NSUInteger)theIndex {
    [textureCollections removeObjectAtIndex:theIndex];
    valid = NO;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:TextureManagerChanged object:self];
}

- (NSArray *)textureCollections {
    return textureCollections;
}

- (void)clear {
    [textureCollections removeAllObjects];
    valid = NO;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:TextureManagerChanged object:self];
}

- (Texture *)textureForName:(NSString *)name {
    NSAssert(name != nil, @"name must not be nil");
    
    [self validate];
    
    Texture* texture = [textures objectForKey:name];
    
    if (texture == nil)
        texture = [dummies objectForKey:name];
    
    if (texture == nil) {
        texture = [[Texture alloc] initDummyWithName:name];
        [dummies setObject:texture forKey:name];
        [texture release];
    }
    
    return texture;
}

- (NSArray *)texturesByCriterion:(ETextureSortCriterion)criterion {
    [self validate];
    switch (criterion) {
        case TS_NAME:
            return texturesByName;
        case TS_USAGE: {
            NSMutableArray* texturesByUsage = [[NSMutableArray alloc] initWithArray:[textures allValues]];
            [texturesByUsage sortUsingSelector:@selector(compareByUsageCount:)];
            return [texturesByUsage autorelease];
        }
        default:
            [NSException raise:NSInvalidArgumentException format:@"unknown sort criterion: %i", criterion];
            break;
    }
    
    return nil; // unreachable
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
    [dummies release];
    [super dealloc];
}

@end
