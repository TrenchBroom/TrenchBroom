//
//  TextureCollection.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Wad;
@class Texture;

@interface TextureCollection : NSObject {
    @private
    NSMutableArray* textures;
    NSString* name;
}

- (id)initName:(NSString *)theName palette:(NSData *)thePalette wad:(Wad *)theWad;

- (NSString *)name;
- (NSArray *)textures;

@end
