//
//  Texture.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Texture.h"
#import "IdGenerator.h"

@implementation Texture

- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight textureId:(int)theTextureId {
    if (self = [self init]) {
        uniqueId = [[[IdGenerator sharedGenerator] getId] retain];
        name = [theName retain];
        textureId = theTextureId;
        width = theWidth;
        height = theHeight;
        usageCount = 0;
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (NSNumber *)uniqueId {
    return uniqueId;
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

- (void)deactivate {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)incUsageCount {
    usageCount++;
}

- (void)decUsageCount {
    usageCount--;
}

- (int)usageCount {
    return usageCount;
}

- (NSComparisonResult)compareByName:(Texture *)texture {
    return [name compare:[texture name]];
}

- (NSComparisonResult)compareByUsageCount:(Texture *)texture {
    if (usageCount > [texture usageCount])
        return NSOrderedAscending;
    if (usageCount < [texture usageCount])
        return NSOrderedDescending;
    return [self compareByName:texture];
}

- (void)dealloc {
    [uniqueId release];
    [name release];
    [super dealloc];
}
@end
