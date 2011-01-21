//
//  TextureManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Wad;

@interface TextureManager : NSObject {
    @private
    NSMutableDictionary* textures;
}

- (void)loadTexturesFrom:(Wad *)wad;

@end
