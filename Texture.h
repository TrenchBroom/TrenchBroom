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
    int usageCount;
}

- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight textureId:(int)theTextureId;

- (NSString *)name;
- (int)textureId;
- (int)width;
- (int)height;

- (void)activate;
- (void)deactivate;

- (void)incUsageCount;
- (void)decUsageCount;
- (int)usageCount;

- (NSComparisonResult)compareByName:(Texture *)texture;
- (NSComparisonResult)compareByUsageCount:(Texture *)texture;
@end
