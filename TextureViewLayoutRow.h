//
//  TextureViewRow.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Texture;

@interface TextureViewLayoutRow : NSObject {
    @private
    float outerMargin;
    float innerMargin;
    float y;
    float width;
    float height;
    NSMutableArray* cells;
}
- (id)initAtY:(float)yPos width:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin;

- (BOOL)addTexture:(Texture *)texture;
- (NSArray *)cells;

- (float)y;
- (float)height;
@end
