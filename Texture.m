//
//  Texture.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Texture.h"


@implementation Texture

- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight textureId:(int)theTextureId {
    if (theName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    if (self = [self init]) {
        name = [theName retain];
        textureId = theTextureId;
        width = theWidth;
        height = theHeight;
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (int)textureId {
    return textureId;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (void)activate {
    glBindTexture(GL_TEXTURE_2D, textureId);
}

- (NSComparisonResult)compare:(Texture *)texture {
    return [name compare:[texture name]];
}

- (void)dealloc {
    [name release];
    [super dealloc];
}
@end
