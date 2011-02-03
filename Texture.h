//
//  Texture.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Texture : NSObject {
    NSString* name;
    int textureId;
    int width;
    int height;
}

- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight textureId:(int)theTextureId;

- (NSString *)name;
- (int)textureId;
- (int)width;
- (int)height;

- (void)activate;
- (void)deactivate;

- (NSComparisonResult)compare:(Texture *)texture;
@end
