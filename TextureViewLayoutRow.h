//
//  TextureViewRow.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Texture;
@class TextureViewLayoutCell;

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

- (BOOL)addTexture:(Texture *)texture nameSize:(NSSize)theNameSize;
- (NSArray *)cells;

- (float)y;
- (float)height;

- (BOOL)containsY:(float)yCoord;
- (TextureViewLayoutCell *)cellAt:(NSPoint)location;
@end
