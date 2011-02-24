//
//  TextureViewCell.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Texture;

@interface TextureViewLayoutCell : NSObject {
    @private
    Texture* texture;
    NSRect textureRect;
    NSRect cellRect;
    NSRect nameRect;
}

- (id)initAt:(NSPoint)location texture:(Texture *)theTexture nameSize:(NSSize)theNameSize;

- (NSRect)cellRect;
- (NSRect)textureRect;
- (NSRect)nameRect;

- (BOOL)contains:(NSPoint)point;

- (Texture *)texture;

@end
