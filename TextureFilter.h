//
//  TextureFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Texture.h"

@protocol TextureFilter<NSObject>

- (BOOL)passes:(Texture *)texture;

@end
