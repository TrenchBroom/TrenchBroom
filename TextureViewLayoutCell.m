//
//  TextureViewCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureViewLayoutCell.h"
#import "Texture.h"

@implementation TextureViewLayoutCell

- (id)initAt:(NSPoint)location texture:(Texture *)theTexture nameSize:(NSSize)theNameSize {
    if (theTexture == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture must not be nil"];
    
    if (self = [self init]) {
        texture = [theTexture retain];
             
        cellRect = NSMakeRect(location.x, location.y, fmax([texture width], theNameSize.width), [texture height] + theNameSize.height + 2);
        textureRect = NSMakeRect(location.x + (cellRect.size.width - [texture width]) / 2, location.y, [texture width], [texture height]);
        nameRect = NSMakeRect(location.x + (cellRect.size.width - theNameSize.width) / 2, location.y + [texture height] + 1, theNameSize.width, theNameSize.height);
    }
    
    return self;
}

- (NSRect)cellRect {
    return cellRect;
}

- (NSRect)textureRect {
    return textureRect;
}

- (NSRect)nameRect {
    return nameRect;
}

- (BOOL)contains:(NSPoint)point {
    return point.x >= cellRect.origin.x && 
           point.x <= cellRect.origin.x + cellRect.size.width && 
           point.y >= cellRect.origin.y && 
           point.y <= cellRect.origin.y + cellRect.size.height;
}

- (Texture *)texture {
    return texture;
}

- (void)dealloc {
    [texture release];
    [super dealloc];
}

@end
